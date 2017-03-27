//
//  ObjectBridge.h
//  For Test
//
//  Created by Brent on 15-3-15. 39386078@qq shensun@cisco
//  Copyright (c) 2014 ??? . NO rights reserved.
//

#ifndef __OBJC_BASECLASS__
#define __OBJC_BASECLASS__

#include <objc/message.h>
#include <objc/runtime.h>

#define OBJC_BLOCK(x)                   x=[^
#define OBJC_BLOCK_END                  copy]
#define OBJC_BLOCK_CLEAR(x)             [x release],x=nil
#define PA(B)                           @encode(B)

#define OBJECTC_ADD_PROTOCOLS(x)        AddProtocol(self, @protocol(x) );

#define OBJC_ADD_BLOCK(m,y)             AddBlock(self,@selector(m),(id)y,0);
#define OBJC_ADD_BLOCK1(m,y,n1)         AddBlock(self,@selector(m),(id)y,1,n1);
#define OBJC_ADD_BLOCK2(m,y,n1,n2)      AddBlock(self,@selector(m),(id)y,2,n1,n2);
#define OBJC_ADD_BLOCK3(m,y,n1,n2,n3)   AddBlock(self,@selector(m),(id)y,2,n1,n2,n3);
#define OBJC_ADD_BLOCKN(m,y,n,nx)       AddBlock(self,@selector(m),(id)y,n,nx);

#define OBJC_ADD_METHOD(m,y)            AddMethod(self,@selector(m),(IMP)&y,0);
#define OBJC_ADD_METHOD1(m,y,n1)        AddMethod(self,@selector(m),(IMP)&y,1,PA(n1));
#define OBJC_ADD_METHOD2(m,y,n1,n2)     AddMethod(self,@selector(m),(IMP)&y,2,PA(n1),PA(n2));
#define OBJC_ADD_METHOD3(m,y,n1,n2,n3)  AddMethod(self,@selector(m),(IMP)&y,1,PA(n1),PA(n2),PA(n3));
#define OBJC_ADD_METHODN(m,y,n,nx)      AddMethod(self,@selector(m),(IMP)&y,n,nx);

#define SELF_ASSOCIATED_KEY             "[CLS_PTR]"

#define OBJC_SUPER_BASE                 ObjectBridge
#define OBJC_METHOD                     static
#define OBJC_CLASS(c,o)                 c:public OBJC_SUPER_BASE<c,o>
#define OBJC_BASE_CLASS(c,b,o)          c:public b<c,o>


#define BEGIN_OBJC_MAP()                id init(id self){
#define END_OBJC_MAP()                  return self;}

template <typename TNewClass,typename BaseObjCClass>
struct BridgeBase
{
public:
    OBJC_METHOD id init(id self ,SEL selector){
        TNewClass* pThis = GetThis(self);
        return pThis->init(self);
    }
    
    OBJC_METHOD id alloc(){
        
        Class clsObj;
        const char* name = typeid(TNewClass).name();
        if((clsObj = objc_lookUpClass(name)) == nil){
            clsObj = objc_allocateClassPair([BaseObjCClass class], name, 0);
            objc_registerClassPair(clsObj);
        }
        
        id self = class_createInstance(clsObj, 0);//[clsObj alloc];
        if( self){
            [self retain];
            objc_setAssociatedObject(self, (void*)SELF_ASSOCIATED_KEY, (id)(new TNewClass()), OBJC_ASSOCIATION_ASSIGN);
            OBJC_ADD_METHOD(init, init);
            OBJC_ADD_METHOD(release, Release);
        }
        return self;
    }
    
protected:
    OBJC_METHOD void Release(id self ,SEL selector){
        TNewClass* pThis = GetThis(self);
        NSInteger nCount = [self retainCount];
        pThis->SuperRun( (id)self, @selector(release) );
        if( 1 == nCount ){
            object_dispose(self);
            objc_disposeClassPair( object_getClass(self) );
            delete (TNewClass*)pThis;
        }
    }
    
    OBJC_METHOD TNewClass* GetThis(id self) {
        return self?(TNewClass*)objc_getAssociatedObject(self, (void*)SELF_ASSOCIATED_KEY):nullptr;
    }
    
public:
    template<typename R>
    OBJC_METHOD R SuperRunStret(id self,SEL selector, ...){
        struct objc_super mySuper = {
            .receiver = self,
            .super_class = class_isMetaClass(object_getClass(self))?object_getClass([self superclass]):[self superclass],
        };
        
        R ret;
        va_list ap;
        va_start (ap, selector);
        ret = (( R(*)(struct objc_super *super, SEL, ...))objc_msgSendSuper_stret)(&mySuper, selector, ap);
        va_end(ap);
        return ret;
    }
    
    /*
     OBJC_METHOD float SuperRunF(id self,SEL selector, ...){
     float ret;
     va_list ap;
     va_start (ap, selector);
     ret = (float)objc_msgSend_fpret(self, selector, ap);
     va_end(ap);
     return ret;
     }*/
    
    OBJC_METHOD id SuperRun(id self,SEL selector, ...){
        id idRet;
        va_list ap;
        va_start (ap, selector);
        
        struct objc_super mySuper = {
            .receiver = self,
            .super_class = class_isMetaClass(object_getClass(self))?object_getClass([self superclass]):[self superclass],
        };
        
        idRet = objc_msgSendSuper(&mySuper, selector,ap);
        va_end (ap);
        return idRet;
    }
    
    OBJC_METHOD BOOL AddProtocol(id pSelf,Protocol* Proto){
        BOOL bRet = YES;
        if ( ![pSelf conformsToProtocol:Proto] )
            bRet = class_addProtocol( object_getClass( pSelf ), Proto);
        return bRet;
    }
    
    OBJC_METHOD BOOL AddBlock(id pSelf,SEL selector,id impBlock,uint Count, ...){
        BOOL bRet = NO;
        va_list ap;
        va_start (ap, Count);
        bRet = AddMethod(pSelf,selector,imp_implementationWithBlock(impBlock),Count,ap);
        va_end (ap);
        return bRet;
    }
    
    OBJC_METHOD BOOL AddMethod(id pSelf,SEL selector , IMP imp , uint Count, ...){
        BOOL bRet = NO;
        if(sel_isMapped(selector) == 0)
            sel_registerName( sel_getName(selector) );
        
        char szCoding[512] = "v@:";
        if( Count > 0){
            char szFormat[512] = {0};
            for(uint i=0; i<Count; i++)
                strcat(szFormat,"%s");
            
            va_list ap;
            va_start (ap, Count);
            sprintf(szCoding+strlen(szCoding),szFormat,ap);
            va_end (ap);
        }
        bRet = class_addMethod(object_getClass( pSelf ),selector,imp,szCoding);
        return bRet;
    }
};


template <typename NewClass,typename BaseObjClass>
struct OBJC_SUPER_BASE:
    public BridgeBase<NewClass,BaseObjClass>
{
public:
protected:
protected:
    
};


#endif
