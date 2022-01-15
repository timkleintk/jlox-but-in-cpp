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
		if (!m_ptr.lock())
		{
			// nts: better error message
			throw std::exception("didn't call setShared();");
		}
		return m_ptr.lock();
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
