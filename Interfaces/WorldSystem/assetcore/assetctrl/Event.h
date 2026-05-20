#pragma once
#include "EventArgs.h"

#include "class/class.h"

#include "AssetCtrlDefines.h"

class CEvent
{
public:
	DEFINE_CLASS(CEvent);
	/*!
	\brief
	Defines abstract interface which will be used when constructing various
	functor objects that bind slots to signals (or in CEGUI terms, handlers to
	events).
	*/
	class CSlotFunctorBase
	{
	public:
		virtual ~CSlotFunctorBase() {};
		virtual BOOL operator()(const WindowEventArgs& args) = 0;
	};

	/*!
	\brief
	Slot template class that creates a functor that calls back via a class
	member function.
	*/
	template<typename T>
	class CMemberFunctionSlot : public CSlotFunctorBase
	{
	public:
		//! Member function slot type.
		typedef BOOL(T::*MemberFunctionType)(const WindowEventArgs&);

		CMemberFunctionSlot(MemberFunctionType func, T* obj) :
			_function(func),
			_object(obj)
		{}

		virtual BOOL operator()(const WindowEventArgs& args)
		{
			return (_object->*_function)(args);
		}

	private:
		MemberFunctionType _function;
		T* _object;
	};

	/*!
	\brief
	SubscriberSlot class which is used when subscribing to events.

	For many uses, the construction of the SubscriberSlot may be implicit, so
	you do not have to specify Subscriber in your subscription calls. Notable
	exceptions are for subscribing member functions and references to functor
	objects.
	*/
	class CSubscriberSlot
	{
	public:
		/*!
		\brief
		Default constructor.  Creates a SubscriberSlot with no bound slot.
		*/
		CSubscriberSlot() : _functor_impl(0) { }

		/*!
		\brief
		Destructor.  Note this is non-virtual, which should be telling you not
		to sub-class!
		*/
		~CSubscriberSlot() {}

		/*!
		\brief
		Invokes the slot functor that is bound to this Subscriber.  Returns
		whatever the slot returns, unless there is not slot bound when FALSE is
		always returned.
		*/
		BOOL operator()(const WindowEventArgs& args) const
		{
			return (*_functor_impl)(args);
		}

		/*!
		\brief
		Returns whether the SubscriberSlot is internally connected (bound).
		*/
		BOOL Connected() const
		{
			return _functor_impl != 0;
		}

		/*!
		\brief
		Disconnects the slot internally and performs any required cleanup
		operations.
		*/
		void Cleanup()
		{
			delete _functor_impl;
			_functor_impl = 0;
		}

		// templatised constructors
		/*!
		\brief
		Creates a SubscriberSlot that is bound to a member function.
		*/
		template<typename T>
			CSubscriberSlot(BOOL (T::*function)(const WindowEventArgs&), T* obj) :
		_functor_impl(new CMemberFunctionSlot<T>(function, obj))
		{}

		/*!
		\brief
		Creates a SubscriberSlot that is bound to a functor pointer.
		*/
		template<typename T>
			CSubscriberSlot(T* functor) :
		_functor_impl(new CFunctorPointerSlot<T>(functor))
		{}

	private:
		//! Points to the internal functor object to which we are bound
		CSlotFunctorBase* _functor_impl;
	};

public:
	/*!
	\brief
		Subscriber object type.  This is now just a typedef to SubscriberSlot,
		the use of the name Event::Subscriber is maintained for hostorical and
		compatability reasons.
	*/
	typedef CSubscriberSlot Subscriber;

    /*!
    \brief
        Constructs a new Event object with the specified id
    */
    CEvent();

    /*!
    \brief
        Destructor for Event objects.  Note that this is non-virtual and so you
        should not sub-class Event.
    */
    ~CEvent();

	void SetID(int id)	{		_id=id;	}
    /*!
    \brief
        Return the id given to this Event object when it was created.

    \return
        int object containing the id of the Event object.
    */
    int GetID(void) const { return _id; }

    /*!
    \brief
        Subscribes some function or object to the Event

    \param subscriber
        A function, static member function, or function object, with the
        signature void function_name(const WindowEventArgs& args).  To subscribe a
        member function you should explicitly create an Event::Subscriber as
        this parameter.

    \return
        Nothing
    */
    void Subscribe(const Subscriber& slot);

    /*!
    \brief
        Fires the event.  All event subscribers get called in the appropriate
        sequence.

    \param args
        An object derived from WindowEventArgs to be passed to each event subscriber.
        The 'handled' field will be set to TRUE if any of the called subscribers
        return that they handled the event.

    \return
        Nothing.
    */
    void operator()(WindowEventArgs& args);


private:
    // Copy constructor and assignment are not allowed for events
    CEvent(const CEvent& other) {}
    CEvent& operator=(const CEvent& other)  {return *this;}

private:
	Subscriber _slot;	//!< Collection holding ref-counted bound slots
    int _id;			//!< Id of this event
};

