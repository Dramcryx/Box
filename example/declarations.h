#pragma once

#include "../Box.h"
#include <cstddef>

struct SizedSomething {
    std::size_t Member = 1;
};

struct IBoxable {
    virtual ~IBoxable() = default;

    virtual int WriteBoxed() = 0;
};

class Boxable : public SizedSomething, public virtual IBoxable {
public:
    Boxable() = default;

    Boxable(Boxable&&) noexcept = default;

    ~Boxable() override = default;

    int WriteBoxed() override;
};

class Boxable2 : public virtual IBoxable {
public:
    Boxable2() = default;

    Boxable2(Boxable2&&) noexcept = default;

    ~Boxable2() override = default;

    int WriteBoxed() override;
};

class VirtualBoxed : public Boxable, public Boxable2 {
public:
    VirtualBoxed() = default;

    VirtualBoxed(VirtualBoxed&&) noexcept = default;

    ~VirtualBoxed() override = default;

    int WriteBoxed() override;
};

template <>
struct dramcryx::BoxSize<IBoxable> {
    static constexpr std::size_t Size = 48;
    static constexpr std::size_t Alignment = 8;
};

dramcryx::Box<IBoxable> BoxCreator();
