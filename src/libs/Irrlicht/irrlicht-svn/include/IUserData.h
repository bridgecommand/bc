// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef IRR_I_USER_DATA_H_INCLUDED
#define IRR_I_USER_DATA_H_INCLUDED

#include "irrTypes.h"

namespace irr
{
namespace io
{
	class IAttributes;
	struct SAttributeReadWriteOptions;
	
	//! Irrlicht may allow users to set their own data via those pointers
	//! Irrlicht has no memory control over IUserData, the user is completely responsible for that
	class IUserData
	{
		public:
		
			//! To be overloaded if comparisons matter
			//! For example in SMaterial you can trigger a material change by returning false
			//! Remember you can cast 'other' inside to your derived class.
			//! Like when you have class MyData : public IUserData you'll 
			//! probably do: const MyData& otherM = static_cast<const MyData&>(other);
			virtual bool compare(const IUserData& other) const
			{
				return true;
			}
			
			//! Used internally by Irrlicht to check if data has changed
			bool operator!=(const IUserData& other) const
			{
				return !compare(other);
			}
			
		protected:
			// Irrlicht is never allowed to delete this
			// If users want to delete such objects they should go over derived classes
			~IUserData() {}
	};
	
} // end namespace io
} // end namespace irr

#endif // IRR_I_USER_DATA_H_INCLUDED
