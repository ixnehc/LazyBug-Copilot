
#pragma warning(disable:4311)

template <typename T> 
struct EventFunc
{
	unsigned __int32 this_address;
	unsigned __int32 func_address;

	_Object *This;
	T Func;

	EventFunc()
	{   
		this_address=offsetof(EventFunc,This);
		this_address+=(unsigned __int32)this;

		func_address=offsetof(EventFunc,Func);
		func_address+=(unsigned __int32)this;
	}
};


#define SetEventCall(event_obj,event_func)      \
{                                                           \
	unsigned __int32 this_address=event_obj.this_address;                                                           \
	unsigned __int32 func_address=event_obj.func_address;                                                          \                                                         
	{                                                               \
		__asm mov eax,this_address               \                                               
		__asm mov ebx,this                               \                               
		__asm mov [eax],ebx                              \                               
		__asm mov eax,func_address                 \                                             
		__asm mov ebx,event_func                      \                                          
		__asm mov [eax],ebx                                 \                        
	}                                                       \
}

#define CallEvent(event_obj)                    (event_obj.This->*(event_obj.Func))             //(*(event_obj.This).*(event_obj.Func))

#define DefineEvent(name,result,intro)          EventFunc<result (_Object:: *)intro> name;


//
//class Button
//{
//public:
//
//	DefineEvent(onClick,void,(Button *,int));     //定义事件,原型为: void onClick(Button *,int)
//
//public:
//
//	Button()
//	{
//		printf("Button this=%p\n",this);
//	}
//
//	void TestButtonClick()
//	{
//		CallEvent(onClick)(this,0);               //呼叫onClick,原型为: onClick(this,0)
//	}
//};
//
//class Test
//{
//	Button *button;
//
//public:
//
//	void OnButtonClick(Button *but,int)
//	{
//		printf("Test::OnButtonClick,this=%p,but=%p\n",this,but);
//	};
//
//public:
//
//	Test()
//	{
//		printf("Test this=%p\n",this);
//
//		button=new Button;
//
//		SetEventCall(button->onClick,OnButtonClick);              //设定button->onClick事件的处理函数为OnButtonClick
//
//		button->TestButtonClick();
//	}
//};
