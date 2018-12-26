// 
// file:		resizablestore.hpp
//

#ifndef __common_resizablestore_hpp__
#define __common_resizablestore_hpp__

#include <stddef.h>

namespace common{

template <typename Type>
class ResizableStore
{
public:
	ResizableStore();
	virtual ~ResizableStore();

	void			resize(size_t newSize);
	void			resize(size_t newSize, void (*a_fpFunc)(Type*));
	size_t			size()const;
	Type*			buffer();
	const Type*		buffer()const;
	Type&			operator[](size_t index);
	const Type&		operator[](size_t index)const;
private:
	Type*		m_ptBuffer;
	size_t		m_unSize;
	size_t		m_unMaxSize;

};

}  // namespace common{

#include "impl.resizablestore.hpp"



#endif  // #ifndef __common_resizablestore_hpp__
