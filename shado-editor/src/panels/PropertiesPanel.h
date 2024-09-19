#pragma once
#include <string>
#include "scene/Entity.h"
#include "script/ScriptEngine.h"
#include "util/Memory.h"

namespace Shado {
    class Scene;
    class Event;

    class PropertiesPanel {
    public:
        PropertiesPanel(const std::string& title = "Properties");
        PropertiesPanel(const Ref<Scene>& scene, const std::string& title = "Properties");

        void setContext(const Ref<Scene>& scene);

        void onImGuiRender();
        void onEvent(Event& e);

        void setSelected(Entity entity);

        Entity getSelected() const { return m_Selected; }
        void resetSelection();

    private:
        void drawComponents(Entity e);

    private:
        Ref<Scene> m_Context;
        Entity m_Selected;
        std::string m_Title;
    };

    /**
     * Classes used to render different script data types for script component
     */
    struct ScriptTypeRendererData {
        Ref<Scene> scene;
        Entity& entity;
        std::string_view fieldName;
        FieldStorage& fieldStorage;
    };

    class ScriptTypeRenderer {
    public:
        ScriptTypeRenderer(DataType type)
            : m_Type(type) {
        }

        virtual ~ScriptTypeRenderer() = default;

        virtual void onImGuiRender(const ScriptTypeRendererData& context) = 0;

        inline DataType getType() const { return m_Type; }

    protected:
        DataType m_Type;
    };

    class ScriptFloatRenderer final : public ScriptTypeRenderer {
    public:
        ScriptFloatRenderer()
            : ScriptTypeRenderer(DataType::Float) {
        }

        virtual void onImGuiRender(const ScriptTypeRendererData& context) override;
    };

    class ScriptIntRenderer final : public ScriptTypeRenderer {
    public:
        ScriptIntRenderer()
            : ScriptTypeRenderer(DataType::Int) {
        }

        virtual void onImGuiRender(const ScriptTypeRendererData& context) override;
    };

    class ScriptBoolRenderer final : public ScriptTypeRenderer {
    public:
        ScriptBoolRenderer()
            : ScriptTypeRenderer(DataType::Bool) {
        }

        virtual void onImGuiRender(const ScriptTypeRendererData& context) override;
    };

    class ScriptVector3Renderer final : public ScriptTypeRenderer {
    public:
        ScriptVector3Renderer()
            : ScriptTypeRenderer(DataType::Vector3) {
        }

        virtual void onImGuiRender(const ScriptTypeRendererData& context) override;
    };

    class ScriptVector4Renderer final : public ScriptTypeRenderer {
    public:
        ScriptVector4Renderer()
            : ScriptTypeRenderer(DataType::Vector4) {
        }

        virtual void onImGuiRender(const ScriptTypeRendererData& context) override;
    };

    class ScriptPrefabRenderer final : public ScriptTypeRenderer {
    public:
        ScriptPrefabRenderer()
            : ScriptTypeRenderer(DataType::Prefab) {
        }

        // TODO: Find a better way to organize this code
        struct PrefabCSMirror {
            uint64_t id;
        };

        virtual void onImGuiRender(const ScriptTypeRendererData& context) override;
    };

    /**
     * All the custom editors defined in C# with [EditorTargetType()] and extends Editor class
     */
    class ScriptCustomEditorRenderer final : public ScriptTypeRenderer {
    public:
        ScriptCustomEditorRenderer()
            : ScriptTypeRenderer((DataType)-1) {
        }

        virtual void onImGuiRender(const ScriptTypeRendererData& context) override;
    };

    ScriptTypeRenderer& GetRendererForType(DataType type);
}
