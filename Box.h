#pragma once

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename TBoxed, std::size_t TBoxedSize, std::size_t TBoxedAlignment, typename TUnboxed>
struct ValidBox {
    static constexpr bool CorrectDeriviation = std::is_base_of_v<TBoxed, TUnboxed>;
    static constexpr bool CastablePointers = std::is_convertible_v<TUnboxed*, TBoxed*>;
    static constexpr bool HasVtable = std::is_polymorphic_v<TUnboxed>;
    static constexpr bool FitsSize = sizeof(TUnboxed) >= sizeof(TBoxed);
    static constexpr bool FitsBox = TBoxedSize >= sizeof(TUnboxed);
    static constexpr bool AlignedWithBox = alignof(TUnboxed) % TBoxedAlignment == 0;
    static constexpr bool IsNoexceptMovable = std::is_nothrow_move_constructible_v<TUnboxed>;

    static constexpr bool Value =
        CorrectDeriviation && CastablePointers && HasVtable && FitsSize &&
        FitsBox && AlignedWithBox && IsNoexceptMovable;
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
        m_cachedPointer = other.m_mover(&other.m_storage, &m_storage);
        m_mover = other.m_mover;
    }

    template<typename TUnboxed, typename ... TArgs>
    inline BoxImpl(BoxInplaceT<TUnboxed>, TArgs&& ... args)
    {
        static_assert(ValidBoxV<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>, "TUnobxed cannot be boxed into TBoxed");

        TUnboxed* unboxed = new (&m_storage) TUnboxed{std::forward<TArgs>(args)...};

        m_cachedPointer = unboxed;

        m_mover = Mover<TUnboxed>;
    }

    template<typename TUnboxed>
    inline BoxImpl(TUnboxed&& unboxedValue)
    {
        static_assert(ValidBoxV<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>, "TUnobxed cannot be boxed into TBoxed");

        TUnboxed* unboxed = new (&m_storage) TUnboxed{std::move(unboxedValue)};

        m_cachedPointer = unboxed;

        m_mover = Mover<TUnboxed>;
    }

    inline ~BoxImpl()
    {
        m_cachedPointer->~TBoxed();
    }

    inline TBoxed* operator->() noexcept
    {
        return m_cachedPointer;
    }

private:
    template<typename TUnboxed>
    static TBoxed* Mover(void* base, void* other) noexcept
    {
        TUnboxed* ptr = new (other) TUnboxed{std::move(*reinterpret_cast<TUnboxed*>(base))};
        return ptr;
    }

    TBoxed* m_cachedPointer = nullptr;
    TBoxedMover* m_mover = nullptr;
    alignas(TBoxedAlignment) unsigned char m_storage[TBoxedSize] {'\0'};
};

template <typename T>
struct BoxSize {
    static constexpr std::size_t Alignment = 0;
    static constexpr std::size_t Size = -1;
};

template <typename TBoxed>
using Box = BoxImpl<TBoxed, BoxSize<TBoxed>::Size, BoxSize<TBoxed>::Alignment>;
