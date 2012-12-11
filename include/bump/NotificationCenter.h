//
//	NotificationCenter.h
//	Bump
//
//	Created by Christian Noon on 12/3/12.
//	Copyright (c) 2012 Christian Noon. All rights reserved.
//

#ifndef BUMP_NOTIFICATION_CENTER_H
#define BUMP_NOTIFICATION_CENTER_H

// Boost headers
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

// Bump headers
#include <bump/Export.h>
#include <bump/NotificationError.h>
#include <bump/String.h>

namespace bump {

/**
 * The base observer class defines the notify() interface as well as contains the
 * observer pointer and the notification name. The notification center uses only
 * Observer objects to handle forwarding all notifications to the appropriate
 * Observer instances.
 */
class BUMP_EXPORT Observer
{
public:

	/**
	 * Defines what type of observer the observer is.
	 */
	enum ObserverType
	{
		KEY_OBSERVER,
		OBJECT_OBSERVER
	};

	/**
	 * Destructor.
	 */
	virtual ~Observer() {}

	/**
	 * Calls the function pointer on the observer instance.
	 */
	virtual void notify() {}

	/**
	 * Calls the function pointer on the observer instance with the given object.
	 *
	 * @param object The object to send to the notification's observer.
	 */
	virtual void notify(const boost::any& object) {}

	/**
	 * Returns the name of the notification that the observer is attached to.
	 *
	 * @return The name of the notification that the observer is attached to.
	 */
	inline const String& notificationName() { return _notificationName; }

	/**
	 * Returns the type of the observer.
	 *
	 * @return The type of the observer.
	 */
	inline const ObserverType& observerType() { return _observerType; }

	/**
	 * Returns whether the given observer is the same as the internal observer.
	 *
	 * param observer An observer pointer to match against the internal observer pointer.
	 * @return True if the internal observer matches the given observer, false otherwise.
	 */
	bool containsObserver(void* observer)
	{
		return observer == _observer;
	}

protected:

	/**
	 * Constructor.
	 */
	Observer() {}

	// Instance member variables
	void*						_observer;			/**< The observer instance used to send notifications. */
	bump::String				_notificationName;	/**< The notification name the observer is observing. */
	ObserverType				_observerType;		/**< The type of observer the observer is. */
};

/**
 * The KeyObserver subclass is used to send an observers notifications without objects based strictly
 * on the key. This subclass is used with the POST_NOIFICATION macro.
 */
template <class T>
class BUMP_EXPORT KeyObserver : public Observer
{
public:

	/**
	 * Constructor.
	 *
	 * @param observer The observer instance used to send notifications.
	 * @param functionPointer The function pointer called on the observer instance when notified.
	 * @param notificationName The name of the notification the observer is observing.
	 */
	KeyObserver(T* observer, void (T::*functionPointer)(), const String& notificationName)
	{
		_observer = observer;
		_functionPointer = boost::bind(functionPointer, observer);
		_notificationName = notificationName;
		_observerType = KEY_OBSERVER;
	}

	/**
	 * Calls the function pointer on the observer instance.
	 */
	virtual void notify()
	{
		_functionPointer();
	}

protected:

	/**
	 * Destructor.
	 */
	~KeyObserver() {}

	// Instance member variables
	boost::function<void ()> _functionPointer;	/**< The function pointer called on the observer instance when notified. */
};

/**
 * The ObjectObserver subclass is used to send an observers notifications with objects based
 * on the key. This subclass is used with the POST_NOIFICATION_WITH_OBJECT macro.
 */
template <class T1, class T2>
class BUMP_EXPORT ObjectObserver : public Observer
{
public:

	/**
	 * Constructor.
	 *
	 * @param observer The observer instance used to send notifications.
	 * @param functionPointer The function pointer called on the observer instance when notified.
	 * @param notificationName The name of the notification the observer is observing.
	 */
	ObjectObserver(T1* observer, void (T1::*functionPointer)(T2), const String& notificationName)
	{
		_observer = observer;
		_functionPointerWithObject = boost::bind(functionPointer, observer, _1);
		_functionPointerWithPointer = NULL;
		_notificationName = notificationName;
		_observerType = OBJECT_OBSERVER;
	}

	/**
	 * Constructor.
	 *
	 * @param observer The observer instance used to send notifications.
	 * @param functionPointer The function pointer called on the observer instance when notified.
	 * @param notificationName The name of the notification the observer is observing.
	 */
	ObjectObserver(T1* observer, void (T1::*functionPointer)(const T2&), const String& notificationName)
	{
		_observer = observer;
		_functionPointerWithObject = boost::bind(functionPointer, observer, _1);
		_functionPointerWithPointer = NULL;
		_notificationName = notificationName;
		_observerType = OBJECT_OBSERVER;
	}

	/**
	 * Constructor.
	 *
	 * @param observer The observer instance used to send notifications.
	 * @param functionPointer The function pointer called on the observer instance when notified.
	 * @param notificationName The name of the notification the observer is observing.
	 */
	ObjectObserver(T1* observer, void (T1::*functionPointer)(T2*), const String& notificationName)
	{
		_observer = observer;
		_functionPointerWithObject = NULL;
		_functionPointerWithPointer = boost::bind(functionPointer, observer, _1);
		_notificationName = notificationName;
		_observerType = OBJECT_OBSERVER;
	}

	/**
	 * Constructor.
	 *
	 * @param observer The observer instance used to send notifications.
	 * @param functionPointer The function pointer called on the observer instance when notified.
	 * @param notificationName The name of the notification the observer is observing.
	 */
	ObjectObserver(T1* observer, void (T1::*functionPointer)(const T2*), const String& notificationName)
	{
		_observer = observer;
		_functionPointerWithObject = NULL;
		_functionPointerWithPointer = boost::bind(functionPointer, observer, _1);
		_notificationName = notificationName;
		_observerType = OBJECT_OBSERVER;
	}

	/**
	 * Calls the function pointer on the observer instance with the given object.
	 *
	 * @throw A bump::NotificationError When the object has an invalid type for the bound callback.
	 *
	 * @param object The object to send to the notification's observer.
	 */
	virtual void notify(const boost::any& object)
	{
		try
		{
			if (_functionPointerWithObject)
			{
				const T2& castObject = boost::any_cast<T2>(object);
				_functionPointerWithObject(castObject);
			}
			else // _functionPointerWithPointer
			{
				T2* castObject = boost::any_cast<T2*>(object);
				_functionPointerWithPointer(castObject);
			}
		}
		catch (const boost::bad_any_cast& /*e*/)
		{
			String msg = String("Notification object for \"%1\" has invalid type for bound callback.").arg(_notificationName);
			throw NotificationError(msg, BUMP_LOCATION);
		}
	}

protected:

	/**
	 * Destructor.
	 */
	~ObjectObserver() {}

	// Instance member variables
	boost::function<void (T2)>	_functionPointerWithObject;		/**< The function pointer that has an object signature. */
	boost::function<void (T2*)>	_functionPointerWithPointer;	/**< The function pointer that has a pointer signature. */
};

/**
 * Central messaging system for passing abstract messages with objects through Bump.
 *
 * The NotificationCenter is a notification system that allows you to send notifications abstractly
 * when events occur in your application. For example, if you're work on an event system, and your event
 * completes, sometimes it would be nice to notify multiple parts of your application that that event
 * was completed. The bump::NotificationCenter makes this type of notification very easy to do. In order
 * to create such a notification, follow these steps:
 *
 * 1) Register all objects as observers with the NotificationCenter
 *
 * @code
 *   bump::AbstractObserver* observer = new bump::ObjectObserver<ObjectType, Event*>(this, &ObjectType::eventCompleted, "EventCompleted");
 *   bump::NotificationCenter::instance()->addObserver(observer);
 *   ADD_OBSERVER(observer); // convenience macro
 * @endcode
 *
 * 2) Make sure to remove the observer from the NotificationCenter in its destructor
 *
 * @code
 *   bump::NotificationCenter::instance()->removeObserver(this);
 *   REMOVE_OBSERVER(observer); // convenience macro
 * @endcode
 *
 * 3) When the event completes, post a notification that the event completed with a matching name
 *
 * @code
 *   bump::NotificationCenter::instance()->postNotificationWithObject("EventCompleted", event);
 *   POST_NOTIFICATION_WITH_OBJECT("EventCompleted", event); // convenience macro
 * @endcode
 *
 * And that's all there is to it! For more information, please see the bumpNotificationCenter example.
 */
class BUMP_EXPORT NotificationCenter
{
public:

	/**
	 * Creates a singleton instance.
	 *
	 * @return The singleton NotificationCenter instance.
	 */
	static NotificationCenter* instance() { static NotificationCenter nc; return &nc; }

	/**
	 * Adds the observer to the list of observers to send notifications.
	 *
	 * @param observer The observer to add to the list of observers to send notifications.
	 */
	void addObserver(Observer* observer)
	{
		if (observer->observerType() == bump::Observer::KEY_OBSERVER)
		{
			_keyObservers.push_back(observer);
		}
		else
		{
			_objectObservers.push_back(observer);
		}
	}

	/**
	 * Determines whether the notification center contains the observer.
	 *
	 * @param observer The observer pointer to check against the list of internal observers stored.
	 * @return True if the observer is registered with the notification center, false otherwise.
	 */
	bool containsObserver(void* observer);

	/**
	 * Calls all observer's function pointers that have registered for the posted notification.
	 *
	 * @param notificationName The notification to post to registered observers.
	 * @return The number of observers that received the notification.
	 */
	unsigned int postNotification(const String& notificationName);

	/**
	 * Calls all observer's function pointers that have registered for the posted notification
	 * with the given object.
	 *
	 * @param notificationName The notification to post to registered observers.
	 * @param object The object to send to the registered observers.
	 * @return The number of observers that received the notification.
	 */
	unsigned int postNotificationWithObject(const String& notificationName, boost::any object);

	/**
	 * Removes the observer from the notification center.
	 *
	 * @param observer The observer instance to remove from the notification center.
	 */
	void removeObserver(void* observer);

protected:

	/**
	 * Constructor.
	 */
	NotificationCenter();

	/**
	 * Destructor.
	 */
	~NotificationCenter();

	// Instance member variables
	std::vector<Observer*> _keyObservers;		/**< The list of key observers registered with the NotificationCenter. */
	std::vector<Observer*> _objectObservers;	/**< The list of object observers registered with the NotificationCenter. */
};

/**
 * Convenience macro for accessing the NotificationCenter singleton.
 */
#define NOTIFICATION_CENTER() bump::NotificationCenter::instance()

/**
 * Convenience macro for accessing the NotificationCenter singleton's addObserver() method.
 */
#define ADD_OBSERVER(c) NOTIFICATION_CENTER()->addObserver(c)

/**
 * Convenience macro for accessing the NotificationCenter singleton's removeObserver() method.
 */
#define REMOVE_OBSERVER(o) NOTIFICATION_CENTER()->removeObserver(o)

/**
 * Convenience macro for accessing the NotificationCenter singleton's postNotification() method.
 */
#define POST_NOTIFICATION(k) NOTIFICATION_CENTER()->postNotification(k)

/**
 * Convenience macro for accessing the NotificationCenter singleton's postNotificationWithObject() method.
 */
#define POST_NOTIFICATION_WITH_OBJECT(k, o) NOTIFICATION_CENTER()->postNotificationWithObject(k, o)

}	// End of bump namespace

#endif	// End of BUMP_NOTIFICATION_CENTER_H
