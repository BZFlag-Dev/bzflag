#pragma once
#include <cstdio>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#define protectfunc /*PROTECTOR_FUNCTION_PRIO=5*/
///////////////////////////////////////////
// protector
///////////////////////////////////////////
class ProtectedFloat {
  protected:
    float unprotected_val;
    virtual float deobfuscate() const { return unprotected_val; };
    virtual void obfuscate(float val) { unprotected_val = val; }

  public:
    ProtectedFloat() { obfuscate(0); }
    ProtectedFloat(float val) { obfuscate(val); }
    virtual ~ProtectedFloat() = default;

    // OPERATORS
    ProtectedFloat &operator=(float val) {
        obfuscate(val);
        return *this;
    }
    ProtectedFloat &operator=(ProtectedFloat val) {
        return operator=(val.deobfuscate());
    }
    ProtectedFloat operator+(ProtectedFloat add) {
        float val = deobfuscate();
        float valadd = add.deobfuscate();
        ProtectedFloat p(val + valadd);
        return p;
    }
    ProtectedFloat operator+(float add) {
        float val = deobfuscate();
        ProtectedFloat p(val + add);
        return p;
    }
    ProtectedFloat &operator+=(float add) {
        obfuscateAdd(add);
        return *this;
    }
    ProtectedFloat &operator++(int) { return this->operator+=(1); }
    ProtectedFloat &operator-=(float add) { return this->operator+=(-add); }
    ProtectedFloat &operator--(int) { return this->operator-=(1); }
    bool operator>=(float comp) { return (deobfuscate() >= comp); }
    bool operator<=(float comp) { return (deobfuscate() <= comp); }
    bool operator>(float comp) { return (deobfuscate() > comp); }
    bool operator<(float comp) { return (deobfuscate() < comp); }
    operator bool() const { return deobfuscate() != 0; }
    // Maybe this can work?
    // operator float() const { return deobfuscate(); }

    float val() { return deobfuscate(); }
    const float val() const { return deobfuscate(); }
    void obfuscateAdd(float add) { obfuscate(deobfuscate() + add); }
    void obfuscateAdd(float add, float linenr) {
        obfuscate(deobfuscate() + add);
    }
};
class SplitFloat : public ProtectedFloat {
  private:
    float val0;
    float val1;
    float val2;
    float val3;
    float deobfuscate() const override { return val0 + val1 + val2 + val3; }
    void obfuscate(float val) override {
        val0 = val - rand();
        val1 = val / 2;
        val2 = val / 3;
        val3 = val - val0 - val1 - val2;
    }

  public:
    SplitFloat() : ProtectedFloat() {}
    SplitFloat(float val) : ProtectedFloat(val) {}
    ~SplitFloat() = default;

    void obfuscateAdd(float add) { val3 += add; }
    void obfuscateAdd(float add, int linenr) {
        if (linenr % 4 == 0) {
            val0 += add;
        } else if (linenr % 4 == 1) {
            val1 += add;
        } else if (linenr % 4 == 2) {
            val2 += add;
        } else if (linenr % 4 == 3) {
            val3 += add;
        }
    }
};

class ProtectedInt {
  protected:
    int unprotected_val;
    virtual int deobfuscate() const { return unprotected_val; };
    virtual void obfuscate(int val) { unprotected_val = val; }

  public:
    ProtectedInt() { obfuscate(0); }
    ProtectedInt(int val) { obfuscate(val); }
    virtual ~ProtectedInt() = default;

    // OPERATORS
    ProtectedInt &operator=(int val) {
        obfuscate(val);
        return *this;
    }
    ProtectedInt &operator=(ProtectedInt val) {
        return operator=(val.deobfuscate());
    }
    ProtectedInt operator+(ProtectedInt add) {
        int val = deobfuscate();
        int valadd = add.deobfuscate();
        ProtectedInt p(val + valadd);
        return p;
    }
    ProtectedInt operator+(int add) {
        int val = deobfuscate();
        ProtectedInt p(val + add);
        return p;
    }
    ProtectedInt &operator+=(int add) {
        obfuscateAdd(add);
        return *this;
    }
    ProtectedInt &operator++(int) { return this->operator+=(1); }
    ProtectedInt &operator-=(int add) { return this->operator+=(-add); }
    ProtectedInt &operator--(int) { return this->operator-=(1); }
    operator bool() const { return deobfuscate() != 0; }
    // Maybe this can work?
    // operator int() const { return deobfuscate(); }

    int val() { return deobfuscate(); }
    void obfuscateAdd(int add) { obfuscate(deobfuscate() + add); }
    void obfuscateAdd(int add, int linenr) { obfuscate(deobfuscate() + add); }
};

class XORInt : public ProtectedInt {
  private:
    int mask = rand();
    int obfuscated_val;
    int deobfuscate() const override { return mask ^ obfuscated_val; }
    void obfuscate(int val) override { obfuscated_val = mask ^ val; }
    void obfuscateAdd(int add) { obfuscated_val += add; }

  public:
    XORInt() : ProtectedInt() {}
    XORInt(int val) : ProtectedInt(val) {}
    ~XORInt() = default;
};

class SplitInt : public ProtectedInt {
  private:
    int val0;
    int val1;
    int val2;
    int val3;
    int deobfuscate() const override { return val0 + val1 + val2 + val3; }
    void obfuscate(int val) override {
        val0 = val - rand();
        val1 = val / 2;
        val2 = val / 3;
        val3 = val - val0 - val1 - val2;
    }

  public:
    SplitInt() : ProtectedInt() {}
    SplitInt(int val) : ProtectedInt(val) {}
    ~SplitInt() = default;

    void obfuscateAdd(int add) { val3 += add; }
    void obfuscateAdd(int add, int linenr) {
        if (linenr % 4 == 0) {
            val0 += add;
        } else if (linenr % 4 == 1) {
            val1 += add;
        } else if (linenr % 4 == 2) {
            val2 += add;
        } else if (linenr % 4 == 3) {
            val3 += add;
        }
    }
};

class ChainInt : public ProtectedInt {
  private:
    int size;
    int **p;
    int **start;
    int deobfuscate() const override { return **p; }
    void obfuscate(int val) override { **p = val; }
    void create_new_path(int val) {
        start = (int **)malloc(sizeof(int *));
        p = start;
        size = rand() % 10 + 1;
        for (int i = 0; i < size; i++) {
            int *next = (int *)malloc(sizeof(int *));
            *p = next;
            p = (int **)next;
        }
        int *next = (int *)malloc(sizeof(int *));
        *p = next;
        **p = val;
    }
    void free_path() {
        // clear value out of memory, might not work thanks to compiler
        obfuscate(0xdeadbeef);
        p = start;
        for (int i = 0; i < size + 1; i++) {
            int *next = *p;
            free(p);
            p = (int **)next;
        }
        free(p);
    }

  public:
    ChainInt() { create_new_path(0); }
    ChainInt(int val) { create_new_path(val); }
    ~ChainInt() { free_path(); }

    void newpath() {
        int val = deobfuscate();
        printf("TEST\n");
        free_path();
        printf("TEST2\n");
        create_new_path(val);
    }
};

class DummyTail {

  public:
    int val;
};

class Dummy {

  public:
    int id;
    Dummy *d1;
    Dummy *d2;
    Dummy *d3;
    DummyTail *tail;
};

class Overseer {
  private:
    // idk i need this function without the if check cus otherwise we cause inf
    // recursion but I also want to keep the other function
    DummyTail *resolve_dummy_interal(Dummy *d) {
        int pick = catalog[d->id];
        Dummy *next = d;
        while (pick != 4) {
            switch (pick) {
            case 0:
                next = next->d1;
                break;
            case 1:
                next = next->d2;
                break;
            case 2:
                next = next->d3;
                break;
            }
            pick = catalog[next->id];
        }
        return next->tail;
    }
    Dummy *create_path_rec(int val, int depth, int max_depth) {
        Dummy *d = new Dummy();
        if (depth < max_depth) {
            d->d1 = create_path_rec(val, depth + 1, max_depth);
            d->d2 = create_path_rec(val, depth + 1, max_depth);
            d->d3 = create_path_rec(val, depth + 1, max_depth);
        }
        d->tail = new DummyTail();
        d->id = rand();
        d->tail->val = rand();
        return d;
    }
    void free_path_rec(Dummy *d) {
        if (d->d1 != nullptr) {
            free_path_rec(d->d1);
            free_path_rec(d->d2);
            free_path_rec(d->d3);
        }
        free(d->tail);
        free(d);
    }

    std::map<int, int> catalog;
    std::vector<int> due_for_path_update;
    int paths_created = 0;

  public:
    Dummy *create_path(int val) {
        // int len = rand();
        int len = 10;
        Dummy *d = create_path_rec(val, 0, len);
        update_path_dummy(d, val);
        paths_created++;
        printf("Created path: %d\n", paths_created);
        return d;
    }

    void free_path(Dummy *d) { free_path_rec(d); }

    DummyTail *resolve_dummy(Dummy *d) {
        // check if dummy is due for chain update
        int index = -1;
        for (int i : due_for_path_update) {
            if (i == d->id) {
                index = i;
            }
        }
        if (index != -1) {
            update_path_dummy(d, resolve_dummy_interal(d)->val);
            due_for_path_update.erase(due_for_path_update.begin() + index);
            printf("Updated path\n");
            // for (const auto &elem : catalog) {
            //     std::cout << elem.first << "," << elem.second << " ";
            // }
            // printf("\n");
        }

        int pick = catalog[d->id];
        Dummy *next = d;
        while (pick != 4) {
            switch (pick) {
            case 0:
                next = next->d1;
                break;
            case 1:
                next = next->d2;
                break;
            case 2:
                next = next->d3;
                break;
            }
            pick = catalog[next->id];
        }
        return next->tail;
    }

    int resolve_dummy_val(Dummy *d) { return resolve_dummy(d)->val; }

    void update_path_dummy(Dummy *d) {
        int val = resolve_dummy_val(d);
        update_path_dummy(d, val);
    }

    void update_path_dummy(Dummy *d, int val) {
        Dummy *curr = d;
        while (curr->d1 != nullptr) {
            int pick = rand() % 3;
            catalog[curr->id] = pick;
            switch (pick) {
            case 0:
                curr = curr->d1;
                break;
            case 1:
                curr = curr->d2;
                break;
            case 2:
                curr = curr->d3;
                break;
            }
        }
        catalog[curr->id] = 4;
        curr->tail->val = val;
    };

    // Could decide here whether to update chain or not based on knowledge of
    // change of value
    void change_value_dummy(Dummy *d, int val) {
        due_for_path_update.push_back(d->id);
        resolve_dummy(d)->val = val;
    }
};
inline Overseer *overseer = new Overseer();

class ChainInt2 : public ProtectedInt {
  private:
    Dummy *path;
    int deobfuscate() const override {
        return overseer->resolve_dummy_val(path);
    }
    void obfuscate(int val) override {
        overseer->change_value_dummy(path, val);
    }

  public:
    ChainInt2() { path = overseer->create_path(0); }
    ChainInt2(int val) { path = overseer->create_path(val); }
    ~ChainInt2() { overseer->free_path(path); }
};

protectfunc inline int get_val_lars(ProtectedInt &val) { return val.val(); }
inline void set_val_lars(ProtectedInt &val, int set) {
    printf("set_val_lars: %d, %d\n", val.val(), set);
    val = set;
}

// inline void add_val_lars(ProtectedInt &val, int add) {
//     printf("add_val_lars: %d, %d\n", val.val(), add);
//     val += add;
// }
