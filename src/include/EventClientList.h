#ifndef EVENT_CLIENT_LIST_H
#define EVENT_CLIENT_LIST_H

#include "common.h"

#include <list>


class EventClient;


//============================================================================//
//============================================================================//

class EventClientList {
  public:
    typedef std::list<EventClient*>::iterator       iterator;
    typedef std::list<EventClient*>::const_iterator const_iterator;

  public:
    EventClientList() : orderType(-1), reversed(false) {}
    EventClientList(int _orderType, bool _reversed)
    : orderType(_orderType)
    , reversed(_reversed)
    {}

    ~EventClientList() {}

    void set_reversed(bool value)  { reversed = value;  }
    void set_order_type(int value) { orderType = value; }

    inline iterator       begin()       { return clients.begin(); }
    inline iterator       end()         { return clients.end();   }
    inline const_iterator begin() const { return clients.begin(); }
    inline const_iterator end()   const { return clients.end();   }
    inline bool           empty() const { return clients.empty(); }
    inline size_t         size()  const { return clients.size();  }

    void clear() { clients.clear(); }

    void purify();

    bool insert(EventClient* value);

    bool remove(const EventClient* value);

  private:
    inline bool lessThan(const EventClient* a, const EventClient* b);

  private:
    std::list<EventClient*> clients;

    int orderType;
    bool reversed;
};


//============================================================================//
//============================================================================//

#endif // EVENT_CLIENT_LIST_H
