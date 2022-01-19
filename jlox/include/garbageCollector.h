#pragma once
#include <memory>
#include <cassert>

template<typename T>
class GarbageCollectable
{
public:
	virtual ~GarbageCollectable() = default;

	std::shared_ptr<T> getShared() const
	{
		auto ptr = m_ptr.lock();
		if (!ptr)
		{
			throw std::exception("called getShared() on an object that was never initialized with setShared().");
		}
		return ptr;
	}

	void setShared(const std::shared_ptr<T>& ptr)
	{
		assert(ptr.get() == this);
		m_ptr = ptr;
	}

private:
	std::weak_ptr<T> m_ptr;
};


template<class T, class... Args>
std::shared_ptr<T> newShared(Args&&... args)
{
	std::shared_ptr<T> ptr = std::make_shared<T>(args...);
	ptr->setShared(ptr);
	return ptr;
}
