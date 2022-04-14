#include "ScriptManager.h"
#include "Debug.h"
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/exception.h>

namespace Shado {

	void ScriptManager::init(const std::string& path) {
        domain = mono_jit_init("my_app");
        assembly = mono_domain_assembly_open(domain, path.c_str());
        if (!assembly)
            SHADO_CORE_ERROR("Error loading assembly");
        image = mono_assembly_get_image(assembly);

        // Add costum functions


        // In case of exception
        mono_install_unhandled_exception_hook([](MonoObject* exc, void* user_data) {
            MonoObject* other = NULL;
            MonoString* str = mono_object_to_string(exc, &other);
            SHADO_CORE_WARN("[C++] exception caught: {0}", mono_string_to_utf8(str));
            throw std::exception("hehexd");
        }, nullptr);
    }

    void ScriptManager::shutdown()
    {
        mono_jit_cleanup(domain);
    }

    void ScriptManager::addInternalCall(const std::string& methodSignature, const void* func) {
        mono_add_internal_call(methodSignature.c_str(), func);
    }

    void ScriptManager::invokeStaticMethod(const std::string& methodSignature, void** args) {
        MonoMethodDesc* desc = mono_method_desc_new(methodSignature.c_str(), false);
        MonoMethod* method = mono_method_desc_search_in_image(desc, image);

        mono_runtime_invoke(method, nullptr, args, nullptr);

        mono_method_desc_free(desc);
    }

    void ScriptManager::invokeInstanceMethod(const std::string& methodSignature, MonoObject* obj, void** args) {
        MonoMethodDesc* desc = mono_method_desc_new(methodSignature.c_str(), false);
        MonoMethod* method = mono_method_desc_search_in_image(desc, image);

        mono_runtime_invoke(method, obj, args, nullptr);
        mono_method_desc_free(desc);
    }

    MonoObject* ScriptManager::createObject(const std::string& klass, const std::string& constructorSignature, void** args) {
        MonoClass* my_class = getClass("Shado", klass.c_str());
        MonoObject* my_class_instance = mono_object_new(domain, my_class);

        // Execute the costructor
        if (args == nullptr && constructorSignature.empty()) {
            mono_runtime_object_init(my_class_instance);
        }
        else
        {
            auto* method = getMethod(constructorSignature.c_str());
            mono_runtime_invoke(method, my_class_instance, args, nullptr);
        }

        return my_class_instance;
    }

    MonoClass* ScriptManager::getClass(const std::string& namesace, const std::string& klass) {
        return mono_class_from_name(image, namesace.c_str(), klass.c_str());
    }

    MonoMethod* ScriptManager::getMethod(const std::string& methodSignature) {
        MonoMethodDesc* desc = mono_method_desc_new(methodSignature.c_str(), false);
        MonoMethod* method = mono_method_desc_search_in_image(desc, image);
        mono_method_desc_free(desc);
        return method;
    }

}