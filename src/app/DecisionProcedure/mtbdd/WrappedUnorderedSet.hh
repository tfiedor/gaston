#ifndef WRAPPEDUNORDEREDSET_H
#define WRAPPEDUNORDEREDSET_H

#include <unordered_set>
#include <iostream>

#include <boost/functional/hash.hpp>

template <class TDataType>
class WrappedUnorderedSet
{
private:
    using DataType = TDataType;
    using SetType = std::unordered_set<DataType>;

public:
    using iterator = typename SetType::iterator;
    using const_iterator = typename SetType::const_iterator;

private:
    SetType set_;
    std::size_t hashValue_;

public:
    WrappedUnorderedSet(): set_(), hashValue_(0)
    { }

    WrappedUnorderedSet(std::initializer_list<DataType> list): set_(list)
    {
        for(auto elem: set_)
            hashValue_ = hashValue_ ^ (uintptr_t)elem;
    }

    inline void insert(const DataType &d)
    {
        auto p = set_.insert(d);

        if(p.second)
            hashValue_ = hashValue_ ^ (uintptr_t)d;
    }

    inline void insert(const DataType &d, int var)
    {
        auto p = set_.insert(d);

        if(p.second)
            hashValue_ = hashValue_ ^ (d->node_ | (var << 26));
    }

    inline void clear()
    {
        set_.clear();
    }


    inline size_t size() const
    {
        return set_.size();
    }

    inline bool empty() const
    {
        return set_.empty();
    }

    inline const_iterator begin() const
    {
        return set_.begin();
    }

    inline const_iterator end() const
    {
        return set_.end();
    }

    inline const_iterator cbegin() const
    {
        return begin();
    }

    inline const_iterator cend() const
    {
        return end();
    }

    bool operator==(const WrappedUnorderedSet& rhs) const
    {
        return (set_ == rhs.set_ && hashValue_ == rhs.hashValue_);
    }

    template <class T>
    friend inline size_t hash_value(const WrappedUnorderedSet<T>& wus);
};

template <class T>
inline size_t hash_value(const WrappedUnorderedSet<T>& wus)
{
    boost::hash<std::size_t> hasher;
    return hasher(wus.hashValue_);
}


#endif // WRAPPEDUNORDEREDSET_H
