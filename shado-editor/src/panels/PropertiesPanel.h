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
        PropertiesPanel( const std::string& title = "Properties");
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
    struct ScriptTypeRendererDataRunning {
        Ref<Scene> scene;
        Entity& entity;
        const std::string& fieldName;
        Ref<ScriptInstance> scriptInstance;
        const std::map<std::string, ScriptField>& scriptClassFields;
    };

    struct ScriptTypeRendererDataStopped {
        Ref<Scene> scene;
        Entity& entity;
        const std::string& fieldName;
        const ScriptField& field;
        Ref<ScriptClass> scriptClass;
        ScriptFieldMap& scriptModifiedFields;
        const std::map<std::string, ScriptField>& scriptClassFields;
    };
    
    class ScriptTypeRenderer {
    public:
        ScriptTypeRenderer(ScriptFieldType type)
            : m_Type(type) {}
        virtual ~ScriptTypeRenderer() = default;
        
        virtual void onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) = 0;
        virtual void onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) = 0;

        inline ScriptFieldType getType() const { return m_Type; }
    protected:
        ScriptFieldType m_Type;
    };

    class ScriptFloatRenderer final : public ScriptTypeRenderer {
    public:
        ScriptFloatRenderer()
            : ScriptTypeRenderer(ScriptFieldType::Float) {}
        
        virtual void onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) override;
        virtual void onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) override;
    };

    class ScriptVector3Renderer final : public ScriptTypeRenderer {
    public:
        ScriptVector3Renderer()
            : ScriptTypeRenderer(ScriptFieldType::Vector3) {}
        
        virtual void onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) override;
        virtual void onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) override;
    };

    class ScriptColourRenderer final : public ScriptTypeRenderer {
    public:
        ScriptColourRenderer()
            : ScriptTypeRenderer(ScriptFieldType::Colour) {}

        virtual void onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) override;
        virtual void onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) override;
    };

    class ScriptPrefabRenderer final : public ScriptTypeRenderer {
    public:
        ScriptPrefabRenderer()
            : ScriptTypeRenderer(ScriptFieldType::Prefab) {}
        
        // TODO: Find a better way to organize this code
        struct PrefabCSMirror {
            uint64_t id;
        };
        
        virtual void onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) override;
        virtual void onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) override;
    };

    /**
     * All the custom editors defined in C# with [EditorTargetType()] and extends Editor class
     */
    class ScriptCustomEditorRenderer final : public ScriptTypeRenderer {
    public:
        ScriptCustomEditorRenderer()
            : ScriptTypeRenderer((ScriptFieldType)-1) {}

        virtual void onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) override;
        virtual void onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) override;
    };
    
    ScriptTypeRenderer& GetRendererForType(ScriptFieldType type);
}
