#ifndef CPPUNIT_TESTRESULT_H
#define CPPUNIT_TESTRESULT_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/SynchronizedObject.h>
#include <deque>

namespace CppUnit {

class Exception;
class Test;
class TestFailure;
class TestListener;

#if CPPUNIT_NEED_DLL_DECL
  template class CPPUNIT_API std::deque<TestListener *>;
#endif

/*! \brief Manages TestListener.
 * \ingroup TrackingTestExecution
 *
 * A single instance of this class is used when running the test. It is usually
 * created by the test runner (TestRunner).
 *
 * This class shouldn't have to be inherited from. Use a TestListener
 * or one of its subclasses to be informed of the ongoing tests.
 * Use a Outputter to receive a test summary once it has finished
 *
 * TestResult supplies a template method 'setSynchronizationObject()'
 * so that subclasses can provide mutual exclusion in the face of multiple
 * threads.  This can be useful when tests execute in one thread and
 * they fill a subclass of TestResult which effects change in another 
 * thread.  To have mutual exclusion, override setSynchronizationObject()
 * and make sure that you create an instance of ExclusiveZone at the 
 * beginning of each method.
 *
 * \see Test, TestListener, TestResultCollector, Outputter.
 */
class CPPUNIT_API TestResult : protected SynchronizedObject
{
public:
  TestResult( SynchronizationObject *syncObject = 0 );
  virtual ~TestResult();

  virtual void addListener( TestListener *listener );
  virtual void removeListener( TestListener *listener );

  virtual void reset();
  virtual void stop();

  virtual bool shouldStop() const;

  virtual void startTest( Test *test );
  virtual void addError( Test *test, Exception *e );
  virtual void addFailure( Test *test, Exception *e );
  virtual void endTest( Test *test );

protected:
  void addFailure( const TestFailure &failure );
  
protected:
  typedef std::deque<TestListener *> TestListeners;
  TestListeners m_listeners;
  bool m_stop;

private: 
  TestResult( const TestResult &other );
  TestResult &operator =( const TestResult &other );
};


} // namespace CppUnit


#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif

#endif // CPPUNIT_TESTRESULT_H


