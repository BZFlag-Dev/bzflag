#ifndef EVENT_CLIENT_LIST_H
#define EVENT_CLIENT_LIST_H

#include "common.h"

#include <vector>

class EventClient;

//============================================================================//
//============================================================================//

class EventClientList {
  public:
    EventClientList() : orderType(-1), reversed(false) {}
    EventClientList(int _orderType, bool _reversed)
    : orderType(_orderType)
    , reversed(_reversed)
    {}

    ~EventClientList() {}

    void set_order_type(int value) {
      orderType = value;
    }

    void set_reversed(bool value) {
      reversed = value;
    }

    inline bool empty() const {
      return data.empty();
    }

    inline bool start(size_t& ptr) {
      ptrs.push_back(&ptr);
      return true;
    }

    inline bool finish() {
      if (ptrs.empty()) {
        return false;
      }
      ptrs.pop_back();
      return true;
    }

    inline bool next(size_t& index, EventClient*& value) {
      if (index >= data.size()) {
        return false;
      }
      value = data[index];
      index++;
      return true;
    }

    void clear() {
      data.clear();
      for (size_t i = 0; i < ptrs.size(); i++) {
        *(ptrs[i]) = 0; // adjust the pointers
      }
    }

    bool insert(EventClient* value);

    bool remove(const EventClient* value);

  private:
    inline void inc_ptrs(size_t index);
    inline void dec_ptrs(size_t index);
    inline bool lessThan(const EventClient* a, const EventClient* b);

  private:
    std::vector<EventClient*> data;
    std::vector<size_t*>      ptrs;

    int orderType;
    bool reversed;
};


//============================================================================//
//============================================================================//

#endif // EVENT_CLIENT_LIST_H
