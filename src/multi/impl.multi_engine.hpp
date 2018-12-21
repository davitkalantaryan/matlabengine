

#ifndef __impl_multi_engine_hpp__
#define __impl_multi_engine_hpp__


#ifndef __matlabengine_multi_engine_hpp__
#error   Do not include this header directly
#include "multi_engine.hpp"
#endif

template <typename Type>
multi::CEngine::ResizableStore<Type>::ResizableStore()
	:
	m_ptBuffer(NULL), 
	m_unSize(0),
	m_unMaxSize(0)
{
}


template <typename Type>
multi::CEngine::ResizableStore<Type>::~ResizableStore()
{
	free(m_ptBuffer);
}


template <typename Type>
void multi::CEngine::ResizableStore<Type>::resize(size_t a_newSize)
{
	if(a_newSize>m_unMaxSize){
		Type* pcTmpBuffer = (Type*)realloc(m_ptBuffer,a_newSize*sizeof(Type));
		if(!pcTmpBuffer){return;}
		m_ptBuffer  = pcTmpBuffer;
		m_unMaxSize = a_newSize;
	}
	m_unSize = a_newSize;
}


template <typename Type>
size_t multi::CEngine::ResizableStore<Type>::size()const
{
	return m_unSize;
}


template <typename Type>
Type* multi::CEngine::ResizableStore<Type>::buffer()
{
	return m_ptBuffer;
}


template <typename Type>
const Type*	multi::CEngine::ResizableStore<Type>::buffer()const
{
	return m_ptBuffer;
}


template <typename Type>
Type& multi::CEngine::ResizableStore<Type>::operator[](size_t a_index)
{
	return m_ptBuffer[a_index];
}


template <typename Type>
const Type& multi::CEngine::ResizableStore<Type>::operator[](size_t a_index)const
{
	return m_ptBuffer[a_index];
}


#endif  // #ifndef __impl_multi_engine_hpp__
