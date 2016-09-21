#pragma once

namespace usdi {

class Attribute
{
public:
    Attribute(Schema *parent, UsdAttribute usdattr);
    virtual ~Attribute();

    UsdAttribute getUSDAttribute() const;
    Schema*     getParent() const;
    const char* getName() const;
    const char* getTypeName() const;
    bool        isArray() const;
    bool        hasValue() const;
    size_t      getNumSamples() const;
    bool        getTimeRange(Time& start, Time& end);

    virtual AttributeType   getType() const = 0;
    virtual size_t          getArraySize(Time t) const = 0; // always 1 if scalar

    // if attribute is array type, *** get()/set() assume dst/src is VtArray<T>*. not T* ***
    // if you want to pass raw pointer, you can use getBuffered()/setBuffered().
    // if attribute is scalar, get()/set() assume dst/src is T* and getBuffered()/setBuffered() just redirect to get()/set().
    virtual bool            get(void *dst, Time t) const = 0;
    virtual bool            set(const void *src, Time t) = 0;
    virtual bool            getBuffered(void *dst, size_t size, Time t) const = 0;
    virtual bool            setBuffered(const void *src, size_t size, Time t) = 0;

protected:
    Schema *m_parent = nullptr;
    UsdAttribute m_usdattr;
    Time m_time_start = usdiInvalidTime, m_time_end = usdiInvalidTime;
    mutable Time m_prev_time = usdiInvalidTime;
#ifdef usdiDebug
    const char *m_dbg_name = nullptr;
    const char *m_dbg_typename = nullptr;
#endif
};

Attribute* WrapExistingAttribute(Schema *parent, UsdAttribute usd);
Attribute* WrapExistingAttribute(Schema *parent, const char *name);
Attribute* CreateNewAttribute(Schema *parent, const char *name, AttributeType type);

} // namespace usdi
