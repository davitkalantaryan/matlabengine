// 
// file:	multi_engine.hpp
//

#ifndef __matlabengine_multi_engine_hpp__
#define __matlabengine_multi_engine_hpp__

#include <engine.h>
#include <stddef.h>
#include <stdint.h>
#include <cpp11+/thread_cpp11.hpp>
#include <common/common_unnamedsemaphorelite.hpp>
#include <common/lists.hpp>
#include <string>
#include <common/common_hashtbl.hpp>

namespace multi {

class CEngine
{
public:
	class EngineTask;
	struct TaskStatus{enum Type{Stopped,Running};};
	CEngine(int a_nEngineNumber);
	virtual ~CEngine();

	int					engineNumber()const;
	void				addFunction2(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex);
	bool				addFunctionIfFree2(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex);
	uint64_t			isRunning()const;
	const EngineTask*	getFirstReadyTask()const;
	void				StartCalc();

protected:
	void				EngineThread();
	void				runFunctionInEngineThread(EngineTask*);

	int					StartEngine();
	void				StopEngine();

protected:
	::Engine*										m_pEngine;
	uint64_t										m_isStarted : 1;
	uint64_t										m_shouldRun : 1;
	uint64_t										m_isError : 1;
	uint64_t										m_isEngineRunning : 1;
	uint64_t										m_nEngineNumber : 24;
	uint64_t										m_nLastFinishedtaskNumber : 24;
	::STDN::thread									m_threadForEngine;
	::common::UnnamedSemaphoreLite					m_semaEngine;
	::common::listN::Fifo<EngineTask*>				m_fifoToDoTasks;
	mutable ::common::listN::Fifo<EngineTask*>		m_fifoDummyTasks;
	mutable ::common::listN::Fifo<EngineTask*>		m_fifoAlreadyDoneTasks;

public:
	template <typename Type>
	class ResizableStore
	{
	public:
		ResizableStore();
		~ResizableStore();

		void			resize(size_t newSize);
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
	class EngineTask
	{
	public:
		int											taskNumber;
		::std::string								funcName;
		ResizableStore<mxArray*>					inputs;
		ResizableStore< ResizableStore<uint8_t*>* >	outputs2;
		int64_t										taskStatus : 3;
	public:
		void			GetOutputs(int a_nNumOuts, mxArray *a_Outputs[])const;
	private:
		friend class CEngine;
		void			resizeOutputs(size_t newSize);
		::common::UnnamedSemaphoreLite*			pSemaToInform;
		//EngineTask(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSema, int a_nIndex);
		EngineTask();
		~EngineTask();
		void init(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSema, int a_nIndex);
	};
};

}

#include "impl.multi_engine.hpp"

#endif  // #ifndef __matlabengine_multi_engine_hpp__
