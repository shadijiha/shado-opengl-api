#pragma once

#ifndef UTIL_H
#define UTIL_H

namespace Shado {
    struct TimeStep {
        TimeStep(double ms) : m_ms(ms) {
        }

        double toMillis() const { return m_ms; }
        double toSeconds() const { return m_ms / 1000; }
        double toFPS() const { return 1.0 / m_ms; }

        operator float() const { return static_cast<float>(m_ms); }
        explicit operator double() const { return m_ms; }

    private:
        double m_ms;
    };
}

#endif
