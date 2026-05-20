

template<class T> CLyObjGrid<T>::CLyObjGrid(void)
{
	_cls = NULL;
	_inst = NULL;
	_ptr = NULL;
	_bLocked = FALSE;
}

template<class T> CLyObjGrid<T>::~CLyObjGrid(void)
{
	if(_cls&&_inst)
		_cls->_del(_inst);
}

template<class T> void CLyObjGrid<T>::Create(CWnd *pParent,DWORD idCtrl)
{
	CRect rc;
	GET_CONTROL_RECT(pParent,idCtrl,rc);
	CGObjGrid::Create(rc,pParent,idCtrl);
}

template<class T> void CLyObjGrid<T>::BindData(const T *pObj)
{
	if(!pObj){
		_ptr = NULL;
		ResetContent();
		return;
	}
	
	//正在编辑时从外界更新
	if(_bLocked)
		return;

	//没有变化
	BOOL bChange = FALSE;
	BOOL bNeedExp = (_ptr==NULL);
	GObjBase * src = (const_cast<T*>(pObj))->GetGObj();
	_cls = (const_cast<T*>(pObj))->GetClass();
	if(!_inst){
		_inst = (T *)_cls->_new();
		bChange = TRUE;		
	}
	else{
		if(_ptr!=pObj||!(_inst->GetGObj()->Equals(src)))
			bChange = TRUE;
	}

	if(bChange){
		LockPaint();
		RecordState(_state);

		_ptr = pObj;
		_inst->GetGObj()->Copy(src);
		CGObjGrid::Bind(_inst->GetGObj());
		if(bNeedExp)		//第一次有数据 展开
			ExpandAll();
		RestoreState(_state);
		UnLockPaint();
	}
}





