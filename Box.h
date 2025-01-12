#pragma once

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace dramcryx {

template <typename TBoxed, std::size_t TBoxedSize, std::size_t TBoxedAlignment, typename TUnboxed>
struct ValidBox {
    static constexpr bool CorrectDeriviation   = std::is_base_of_v<TBoxed, TUnboxed>;
    static constexpr bool CastablePointers     = std::is_convertible_v<TUnboxed*, TBoxed*>;
    static constexpr bool HasVirtualDestructor = std::has_virtual_destructor_v<TUnboxed>;
    static constexpr bool FitsSize             = sizeof(TUnboxed) >= sizeof(TBoxed);
    static constexpr bool FitsBox              = TBoxedSize >= sizeof(TUnboxed);
    static constexpr bool AlignedWithBox       = alignof(TUnboxed) % TBoxedAlignment == 0;
    static constexpr bool IsNoexceptMovable    = std::is_nothrow_move_constructible_v<TUnboxed>;

    static constexpr bool Value =
        CorrectDeriviation &&
        CastablePointers &&
        HasVirtualDestructor &&
        FitsSize &&
        FitsBox &&
        AlignedWithBox &&
        IsNoexceptMovable;
};

template <typename TBoxed, std::size_t TBoxedSize, std::size_t TBoxedAlignment, typename TUnboxed>
static constexpr bool ValidBoxV = ValidBox<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>::Value;

template <typename T>
struct BoxInplaceT {};

template <typename TBoxed, std::size_t TBoxedSize, std::size_t TBoxedAlignment>
class BoxImpl
{
    using TBoxedMover = TBoxed*(void*, void*);

public:
    BoxImpl() = delete;

    inline BoxImpl(BoxImpl&& other) noexcept
    {
        Destroy();

        m_cachedPointer = other.m_mover(&other.m_storage, &m_storage);
        m_mover = other.m_mover;
    }

    inline BoxImpl& operator=(BoxImpl&& other) noexcept
    {
        Destroy();

        m_cachedPointer = other.m_mover(&other.m_storage, &m_storage);
        m_mover = other.m_mover;

        return *this;
    }

    template<typename TUnboxed, typename ... TArgs>
    inline BoxImpl(BoxInplaceT<TUnboxed>, TArgs&& ... args) :
        m_cachedPointer{new (&m_storage) TUnboxed{std::forward<TArgs>(args)...}},
        m_mover{Mover<TUnboxed>}
    {
        static_assert(ValidBoxV<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>, "TUnobxed cannot be boxed into TBoxed");
    }

    template<typename TUnboxed>
    inline BoxImpl(TUnboxed&& unboxedValue) :
        m_cachedPointer{new (&m_storage) TUnboxed{std::move(unboxedValue)}},
        m_mover{Mover<TUnboxed>}
    {
        static_assert(ValidBoxV<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>, "TUnobxed cannot be boxed into TBoxed");
    }

    inline ~BoxImpl()
    {
        Destroy();
    }

    template<typename TUnboxed>
    inline BoxImpl& operator=(TUnboxed&& unboxedValue)
    {
        static_assert(ValidBoxV<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>, "TUnobxed cannot be boxed into TBoxed");

        Destroy();

        m_cachedPointer = new (&m_storage) TUnboxed{std::move(unboxedValue)};

        return *this;
    }

    inline TBoxed* operator->() noexcept
    {
        return m_cachedPointer;
    }

private:
    template<typename TUnboxed>
    static TBoxed* Mover(void* source, void* target) noexcept
    {
        return new (target) TUnboxed{std::move(*reinterpret_cast<TUnboxed*>(source))};
    }

    inline void Destroy()
    {
        if (m_cachedPointer)
        {
            std::exchange(m_cachedPointer, nullptr)->~TBoxed();
        }
    }

    // To trick around MSVC being able to align right introducing left padding,
    // storage for the object goes first.
    //
    alignas(TBoxedAlignment) unsigned char m_storage[TBoxedSize] {'\0'};

    // Casted object pointer in case of multiple inheritance.
    //
    TBoxed* m_cachedPointer = nullptr;

    // Move constructor wrapper.
    //
    TBoxedMover* m_mover = nullptr;
};

template <typename T>
struct BoxSize {
    static constexpr std::size_t Alignment = 0;
    static constexpr std::size_t Size = -1;
};

template <typename TBoxed>
using Box = BoxImpl<TBoxed, BoxSize<TBoxed>::Size, BoxSize<TBoxed>::Alignment>;

} // namespace dramcryx
