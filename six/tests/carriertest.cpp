#undef NDEBUG

#include "misc.h"
#include "carrier.h"

#include <cassert>
#include <algorithm>

using std::cout;
using std::endl;

class CarrierTest
{
public:
  void checkHasFields(const Carrier &c)
  {
    const vector<int> &fields = c.fields();
//     cout << c << endl;
//     for(unsigned i = 0; i < fields.size(); i++) {
//       cout << " " << fields[i];
//     }
//     cout << endl;
    int s = 0;
    for(int i = 0; i < c.limit(); i++) {
      assert((c.has(i) != 0) ==
             (find(fields.begin(), fields.end(), i) != fields.end()));
      if(c.has(i)) {
        s++;
      }
    }
    assert(c.size() == s);
  }

  void checkSize(const Carrier &c)
  {
    assert(c.fields().size() == (unsigned)c.size());
  }

  void check(const Carrier &c)
  {
    checkHasFields(c);
    checkSize(c);
  }

  Carrier createRandomCarrier(int *size = 0)
  {
    Carrier r;
    int s = 0;
    for(int b = 0; b < r.limit(); b++) {
      if(RANDOM(2)) {
        r.addField(b);
        s++;
      }
    }
    if(size)
      *size = s;
    return r;
  }

  void test1()
  {
    {
      Carrier c, d;
      check(c);
      check(d);
      assert(c.empty());
      assert(d.empty());
      cout << "c=" << c << endl;
      c.addField(5);
      c.addField(2);
      cout << "c=" << c << endl;
      cout << c.size() << endl;
      assert(c.size() == 2);
      d.addField(3);
      d.addField(5);
      d.addField(7);
      assert(d.size() == 3);
      cout << "c=" << c << ", size=" << c.size() << endl;
      vector<int> fields = c.fields();
      for(unsigned i = 0; i < fields.size(); i++) {
        if(i) {
          cout << ",";
        }
        cout << fields[i];
      }
      cout << "d=" << d << ", size=" << d.size() << endl;
      Carrier e(c);
      check(e);
      Carrier f(c);
      check(f);

      e.intersect(d);
      check(e);
      cout << "d intersection c =" << e << endl;
      f.unite(d);
      check(f);
      cout << "d union c =" << f << endl;

      assert(f.includes(f));
      assert(f.includes(d));
    }
  }

  void testSizePerf()
  {
    int n = 100000;
    vector<Carrier> v(n);
    for(int i = 0; i < n; i++) {
      v.push_back(createRandomCarrier());
    }
    clock_t t0 = clock();
    for(unsigned i = 0; i < v.size(); i++) {
      v[i].size();
    }
    clock_t t1 = clock();
    cout << "testSizePerf: " << ((long)t1 - (long)t0) / 1000 << "ms" << endl;
  }

  void test2()
  {
    RANDOMSEED(1234);
    for(int i = 0; i < 1000; i++) {
      Carrier c = createRandomCarrier();
      Carrier d = createRandomCarrier();
      Carrier cd = c;
      cd.intersect(d);
      Carrier e(c);
      e.intersect(d);
      assert(e.size() <= c.size());
      e.unite(d);
      assert(e.size() == d.size());
      assert(e == d);
    }
  }

  void test3()
  {
    RANDOMSEED(1234);
    for(int i = 0; i < 1000; i++) {
      Carrier c = createRandomCarrier();
      Carrier d = createRandomCarrier();
      Carrier cd = c;
      cd.intersect(d);
      Carrier e(c);
      e.unite(d);
      assert(e.size() == c.size() + d.size() - cd.size());
      e.intersect(d);
      assert(e == d);
    }
  }

};

int main()
{
  Carrier::init();
  CarrierTest t;
  t.test1();
  t.test2();
  t.test3();
  t.testSizePerf();
  return 0;
}
