# cpp_objc
using objects as cpp

for example:

    struct OBJC_CLASS(DataView,UIViewController)
    {
        virtual ~DataView(){
        }

        public:

        BEGIN_OBJC_MAP()
            OBJC_ADD_METHOD(viewDidLoad,viewDidLoad)
            OBJC_ADD_METHOD(viewCleanImpl,viewCleanImpl)
            OBJC_ADD_METHOD(viewLoadImpl,viewLoadImpl)
            OBJC_ADD_METHOD1(OnFresh:,OnFresh,id)
        END_OBJC_MAP();

        OBJC_METHOD void OnFresh(UIViewController* self ,SEL selector,id sender)
        {
            if( !NET::CheckNetWorkAndShowAlertView(YES) )
                return;

            RightView* pRightView = (RightView*)[self.view viewWithTag:RIGHT_PAN_VIEW];
            if( pRightView ){
                [pRightView Update];
            }
        }

        OBJC_METHOD void viewLoadImpl(UIViewController* self ,SEL selector)
        {
            RightView* pRightView = (RightView*) [self.view viewWithTag:RIGHT_PAN_VIEW];
            if(!pRightView){
                pRightView = [[[RightView alloc]init]autorelease];
                pRightView.tag = RIGHT_PAN_VIEW;
                pRightView.backgroundColor = [UIColor blackColor];
                pRightView.layer.borderWidth = 1;
                pRightView.layer.borderColor = [[UIColor darkGrayColor] CGColor];
                pRightView.clipsToBounds = YES;
                pRightView.frame = self.view.bounds;
                [self.view addSubview:pRightView];
            }
        }
    
        OBJC_METHOD void viewDidLoad(UIViewController* self ,SEL selector)
        {
            DataView* pDataView = GetThis(self);
            pDataView->SuperRun( self,selector );
            self.title = @"Solar Today";
            [self viewLoadImpl];
        }

        OBJC_METHOD void viewCleanImpl(UIViewController* self ,SEL selector)
        {
            RightView* pRightView = (RightView*) [self.view viewWithTag:RIGHT_PAN_VIEW];
            if( pRightView)
                [pRightView removeFromSuperview];
        }
    };

