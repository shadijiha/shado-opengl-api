#pragma once

#include <Coral/ManagedObject.hpp>

#include "debug/Debug.h"

namespace Shado {
    class CSharpObject {
    public:
        template <typename... TArgs>
        void Invoke(std::string_view methodName, TArgs&&... args) {
            SHADO_CORE_ASSERT(m_Handle != nullptr, "");
            m_Handle->InvokeMethod(methodName, std::forward<TArgs>(args)...);
        }

        bool IsValid() const { return m_Handle != nullptr; }

        Coral::ManagedObject* GetHandle() const { return m_Handle; }

    private:
        Coral::ManagedObject* m_Handle = nullptr;

        friend class ScriptEngine;
    };
}
