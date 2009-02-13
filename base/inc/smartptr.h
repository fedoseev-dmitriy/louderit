/** Smart pointer.
* Smart pointer is reference counted pointer. When reference count are <= 0 
* object are automaticly destroyed.
*/
template<typename T> class SmartPtr
{
public:
	typedef T value_type;
	typedef SmartPtr<T> this_type;

	typedef value_type *pointer;
	typedef const value_type *const_pointer;

	typedef value_type &reference;
	typedef const value_type &const_reference;

	/** Default constuctor */
	SmartPtr()
	{
		Object = NULL;
		ReferenceCount = NULL;
	}

	/** Create from pointer */
	SmartPtr(pointer o)
	{
		Object = NULL;
		ReferenceCount = NULL;
		*this = o;
	}

	/** Create from smart pointer */
	SmartPtr(const this_type &p)
	{
		Object = NULL;
		ReferenceCount = NULL;
		*this = p;
	}

	~SmartPtr()
	{
		Release();
	}

	// Assigment
	inline SmartPtr& operator=(pointer o)
	{
		Release();
		Object = o;
		if (Object) AddReference();
		return *this;
	}

	inline SmartPtr& operator=(const this_type &p)
	{
		Release();
		Object = p.Object;
		ReferenceCount = p.ReferenceCount;
		if (Object) AddReference();
		return *this;
	}

	/** Reference to object */
	inline reference operator*()
	{
		return *Object;
	}

	inline /*const_*/reference operator*() const
	{
		return *Object;
	}

	/** Pointer to object */
	inline pointer operator->()
	{
		return Object;
	}

	inline /*const_*/pointer operator->() const
	{
		return Object;
	}

	inline operator pointer*()
	{
		return Object;
	}

	inline operator /*const*/ pointer*() const
	{
		return Object;
	}

	/** Pointer to object */
	inline pointer Get()
	{
		return Object;
	}

	inline /*const_*/pointer Get() const
	{
		return Object;
	}

	/** True if pointer != NULL */
	inline bool operator!() const
	{
		return Object != NULL;
	}

	// Equality
	inline bool operator==(pointer o) const
	{
		return Object == o;
	}

	inline bool operator==(const this_type &p) const
	{
		return Object == p.Object;
	}

	inline bool operator!=(pointer o) const
	{
		return Object != o;
	}

	inline bool operator!=(const this_type &p) const
	{
		return Object != p.Object;
	}

protected:
	/** Internal method for increase reference counter */
	inline void AddReference()
	{
		if (!ReferenceCount) ReferenceCount = new int(0);
		(*ReferenceCount)++;
	}

	/** Internal method for decrease reference counter and destroy object if need */
	inline void Release()
	{
		if (ReferenceCount)
		{
			(*ReferenceCount)--;
			if ((*ReferenceCount) <= 0)
			{
				delete ReferenceCount;
				ReferenceCount = NULL;
				if (Object)
				{
					delete Object;
					Object = NULL;
				}
			}
		}
	}

	/** Pointer to object */
	pointer Object;
	/** Pointer to object reference count */
	int *ReferenceCount;
};