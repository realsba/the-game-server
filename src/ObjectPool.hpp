// file   : ObjectPool.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

#include <memory>
#include <vector>
#include <queue>

namespace details {

template <typename T>
void defaultReseter(T& obj)
{
  obj.reset();
}

} // details

template <typename T, void(*Reseter)(T&)=details::defaultReseter<T>>
class ObjectPool {
public:
  ~ObjectPool()
  {
    clear();
  }

  T* obtain()
  {
    if (m_objects.empty()) {
      return new T();
    }
    T* p(m_objects.front());
    m_objects.pop();
    return p;
  }

  void free(T* obj)
  {
    if (obj == nullptr) {
      return;
    }
    Reseter(*obj);
    m_objects.push(obj);
  }

  void clear()
  {
    while (m_objects.size()) {
      delete m_objects.front();
      m_objects.pop();
    }
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_objects.size();
  }

  std::shared_ptr<T> share()
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_objects.empty()) {
      return std::shared_ptr<T>(new T(), Deleter(*this));
    }
    std::shared_ptr<T> ptr(m_objects.front(), Deleter(*this));
    m_objects.pop();

    return ptr;
  }

protected:
  class Deleter {
  public:
    explicit Deleter(ObjectPool<T, Reseter>& pool) : m_pool(pool) { }

    void operator()(T* p)
    {
      m_pool.free(p);
    }

  protected:
    ObjectPool& m_pool;
  };

  mutable std::mutex  m_mutex;
  std::queue<T*>      m_objects;
};

#endif /* OBJECT_POOL_HPP */
