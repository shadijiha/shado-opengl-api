#pragma once

#include <unordered_map>
#include <string>

#include "CSharpObject.h"
#include "ScriptEntityStorage.hpp"

#include <Coral/Assembly.hpp>
#include <Coral/Type.hpp>
#include <Coral/StableVector.hpp>
#include <Coral/Attribute.hpp>
#include <Coral/Array.hpp>

#include "ui/UUID.h"
#include "util/Buffer.h"

namespace Coral {
    class HostInstance;
    class ManagedAssembly;
    class AssemblyLoadContext;
}

namespace Shado {
    class Scene;
    class Project;

    struct AssemblyData {
        Coral::ManagedAssembly* Assembly;
        std::unordered_map<UUID, Coral::Type*> CachedTypes;
    };

    struct FieldMetadata {
        std::string Name;
        DataType Type;
        Coral::Type* ManagedType;

        Buffer DefaultValue;

    private:
        template <typename T>
        void SetDefaultValue(Coral::ManagedObject& temp) {
            if (ManagedType->IsSZArray()) {
                auto value = temp.GetFieldValue<Coral::Array<T>>(Name);
                DefaultValue = Buffer::Copy(value.Data(), value.ByteLength());
                Coral::Array<T>::Free(value);
            }
            else if (Type == DataType::String) {
                auto value = temp.GetFieldValue<std::string>(Name);
                DefaultValue = Buffer::Copy(value.data(), value.size() + 1);
            }
            else {
                DefaultValue.Allocate(sizeof(T));
                auto value = temp.GetFieldValue<T>(Name);
                DefaultValue.Write(&value, sizeof(T));
            }
        }

        friend class ScriptEngine;
    };

    struct ScriptMetadata {
        std::string FullName;
        std::unordered_map<uint32_t, FieldMetadata> Fields;
    };

    class ScriptEngine {
    public:
        Ref<Scene> GetCurrentScene() const { return m_CurrentScene; }
        void SetCurrentScene(Ref<Scene> scene) { m_CurrentScene = scene; }

        bool IsValidScript(UUID scriptID) const;

        const ScriptMetadata& GetScriptMetadata(UUID scriptID) const;
        const std::unordered_map<UUID, ScriptMetadata>& GetAllScripts() const { return m_ScriptMetadata; }

        const Coral::Type* GetTypeByName(std::string_view name) const;

    public:
        static const ScriptEngine& GetInstance();

    private:
        void InitializeHost();
        void ShutdownHost();

        void Initialize(Ref<Project> project);
        void Shutdown();

        void LoadProjectAssembly();
        void LoadProjectAssemblyRuntime(Buffer data);

        void BuildAssemblyCache(AssemblyData* assemblyData);

        template <typename... TArgs>
        CSharpObject Instantiate(UUID entityID, ScriptStorage& storage, TArgs&&... args) {
            SHADO_CORE_ASSERT(storage.EntityStorage.contains(entityID), "");

            auto& entityStorage = storage.EntityStorage.at(entityID);

            if (!IsValidScript(entityStorage.ScriptID))
                return {};

            auto* type = m_AppAssemblyData->CachedTypes[entityStorage.ScriptID];
            auto instance = type->CreateInstance(std::forward<TArgs>(args)...);
            auto [index, handle] = m_ManagedObjects.Insert(std::move(instance));

            entityStorage.Instance = &handle;

            for (auto& [fieldID, fieldStorage] : entityStorage.Fields) {
                const auto& fieldMetadata = m_ScriptMetadata[entityStorage.ScriptID].Fields[fieldID];

                auto& editorAssignableAttribType = m_CoreAssemblyData->Assembly->GetType(
                    "Hazel.EditorAssignableAttribute");
                if (fieldMetadata.ManagedType->HasAttribute(editorAssignableAttribType)) {
                    Coral::ManagedObject value = fieldMetadata.ManagedType->CreateInstance(
                        fieldStorage.GetValue<uint64_t>());
                    handle.SetFieldValue(fieldStorage.GetName(), value);
                    value.Destroy();
                }
                else if (fieldMetadata.ManagedType->IsSZArray()) {
                    if (fieldMetadata.ManagedType->GetElementType().HasAttribute(editorAssignableAttribType)) {
                        Coral::Array<Coral::ManagedObject> arr = Coral::Array<Coral::ManagedObject>::New(
                            fieldStorage.GetLength());

                        for (int32_t i = 0; i < fieldStorage.GetLength(); i++) {
                            arr[i] = fieldMetadata.ManagedType->GetElementType().CreateInstance(
                                fieldStorage.GetValue<uint64_t>(i));
                        }

                        handle.SetFieldValue(fieldStorage.GetName(), arr);

                        for (int32_t i = 0; i < fieldStorage.GetLength(); i++)
                            arr[i].Destroy();

                        Coral::Array<Coral::ManagedObject>::Free(arr);
                    }
                    else {
                        struct ArrayContainer {
                            void* Data;
                            int32_t Length;
                        } array;

                        array.Data = fieldStorage.m_ValueBuffer.Data;
                        array.Length = static_cast<int32_t>(fieldStorage.GetLength());

                        handle.SetFieldValueRaw(fieldStorage.GetName(), &array);
                    }
                }
                else if (fieldMetadata.Type == DataType::String) {
                    auto s = Coral::String::New(static_cast<const char*>(fieldStorage.m_ValueBuffer.Data));
                    handle.SetFieldValueRaw(fieldStorage.GetName(), &s);
                    Coral::String::Free(s);
                }
                else {
                    handle.SetFieldValueRaw(fieldStorage.GetName(), fieldStorage.m_ValueBuffer.Data);
                }

                fieldStorage.m_Instance = &handle;
            }

            CSharpObject result;
            result.m_Handle = &handle;
            return result;
        }

        void DestroyInstance(UUID entityID, ScriptStorage& storage) {
            SHADO_CORE_ASSERT(storage.EntityStorage.contains(entityID), "");

            auto& entityStorage = storage.EntityStorage.at(entityID);

            SHADO_CORE_ASSERT(IsValidScript(entityStorage.ScriptID), "");

            for (auto& [fieldID, fieldStorage] : entityStorage.Fields)
                fieldStorage.m_Instance = nullptr;

            entityStorage.Instance->Destroy();
            entityStorage.Instance = nullptr;

            // TODO(Peter): Free-list
        }

    private:
        static ScriptEngine& GetMutable();

    private:
        ScriptEngine() = default;

        ScriptEngine(const ScriptEngine&) = delete;
        ScriptEngine(ScriptEngine&&) = delete;

        ScriptEngine& operator=(const ScriptEngine&) = delete;
        ScriptEngine& operator=(ScriptEngine&&) = delete;

    private:
        std::unique_ptr<Coral::HostInstance> m_Host;
        std::unique_ptr<Coral::AssemblyLoadContext> m_LoadContext;
        ScopedRef<AssemblyData> m_CoreAssemblyData = nullptr;
        ScopedRef<AssemblyData> m_AppAssemblyData = nullptr;

        std::unordered_map<UUID, ScriptMetadata> m_ScriptMetadata;

        Ref<Scene> m_CurrentScene = nullptr;
        Coral::StableVector<Coral::ManagedObject> m_ManagedObjects;

    private:
        friend class Application;
        friend class Project;
        friend class Scene;
        friend class SceneHierarchyPanel;
        friend class SceneSerializer;
        friend class EditorLayer;
        friend class PropertiesPanel;
    };
}
