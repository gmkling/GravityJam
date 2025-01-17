#include "GravityJamRoot.h"
#include "GJ_Settings.h"

USING_NS_CC;

Scene* GravityJamRoot::createScene()
{
    // 'scene', 'layer' are autorelease objects
    Scene * scene = Scene::create();
    auto layer = GravityJamRoot::create();
    scene->retain();
    layer->retain();
    scene->addChild(layer);

    // return the scene
    return scene;
}

bool GravityJamRoot::init()
{
    // super init()
    if ( !Layer::init() )
    {
        return false;
    }
    
    // cache some data from the director, set bkgnd color
    
    visibleSize = Director::getInstance()->getVisibleSize();
        origin = Director::getInstance()->getVisibleOrigin();
    
    Color3B glClear = GJ_Settings::gj_backgroundUIColor;
    glClearColor(glClear.r,glClear.g,glClear.b, 0);
    
    gj_level = 1;
    faulted = false;
    
    // TO DO ---> get our resolution/scaling straight for different devices   

    
    /*
     * Set up the main menu scene and transfer control there.
     *
     */
    
    Director::getInstance()->pushScene(MainMenu::createScene());
    this->scheduleUpdate();
    
    return true;
}

bool GravityJamRoot::initHUD()
{
  
    // bring up the HUD
    theHUD.init();
    theHUD.theScore.setScore(this->theScore.getScore());
    this->addChild(&theHUD);
    
    return true;
}

bool GravityJamRoot::initLevel(int levelNum)
{
    // Is there anything we need to do before loading the level? The Level class should handle most of it.
//    if(theLevel.loadLevel(levelNum, this))
    if(theLevel.loadLevel(levelNum))
    {
        return true;
    }
    printf("Error opening tmx file.\n");
    return false;
}

void GravityJamRoot::closeLevel(void)
{
    // To do: what needs to happen between levels?
    // probably unload level data, update event listeners, etc.
    
    theLevel.unloadLevel();
}

bool GravityJamRoot::nextLevel(void)
{
    gj_level++;
    
    // we should check the level against a max, but I don't know how to determine that yet.
    // for now, we'll let that fail happen in Level::initLevel()
    return initLevel(gj_level);
}

void GravityJamRoot::endLevel(float dt)
{
    // we will need all of the end of level graphics here
    // for now, just unload the level and load the next one
    closeLevel();
    if(!nextLevel())
    {
        printf("Error opening level %i", gj_level);
        faulted = true;
    }
}

bool GravityJamRoot::setupTouches()
{
    // set up touch listening - one at a time
    touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->retain();
    
    touchListener->onTouchBegan = [this](Touch* touch, Event* event)
    {
        
        // get the node out of the event, cast it to Node
        auto target = static_cast<Node*>(event->getCurrentTarget());
        
        //Get the position of the current point relative to node space of this
        Vec2 locationInNode = target->convertToNodeSpace(touch->getLocation());
        Size s = target->getContentSize();
        Rect rect = Rect(0, 0, s.width, s.height);
        
        //Check the click area
        if (rect.containsPoint(locationInNode))
        {
            // set the touch starting point
            this->touchBegin.set(locationInNode);
            return true;
        }
        return false;
    };
    
    touchListener->onTouchMoved = [](Touch* touch, Event* event){
        // Do nothing. perhaps modify a compass to show swipe direction?
    };
    
    
    //Process the touch end event
    touchListener->onTouchEnded = [&](Touch* touch, Event* event){
        // reject touch ended if we are in a faulted state so we don't crash
        // alternatively, we should disable the listener and prompt the user to return to main menu, etc.
        if (faulted) {
            return;
        }
        auto target = static_cast<Node*>(event->getCurrentTarget());
        Vec2 locationInNode = target->convertToNodeSpace(touch->getLocation());
        this->touchEnd.set(locationInNode);
        
        //
        int dir = this->getTouchDirection();
        CCLOG("Direction detected: %i \n", dir);
        
        theLevel.moveTiles(dir);
        
        // check for end of level condition, additional moves
        theLevel.endOfMoveChecks(dir);
        
    };
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    return true;
}

int GravityJamRoot::getTouchDirection()
{
    // **** need to add a policy for extra short gestures?
    
    Vec2 touchVector = Vec2(touchBegin, touchEnd);
    float swipeX = fabsf(touchEnd.x - touchBegin.x);
    float swipeY = fabsf(touchEnd.y - touchBegin.y);
    
    
    if(swipeX>swipeY) /* swipe left/right */
    {
        if (touchEnd.x>touchBegin.x) {
            // right swipe
            return RIGHT;
        } else {
            // left swipe
            return LEFT;
        }
        
    } else { /* swipe up/down */
        
        if (touchEnd.y>touchBegin.y) {
            // up swipe
            return UP;
        } else {
            // down swipe
            return DOWN;
        }
    }
    
    // default to 0==NO direction
    return NO_DIRECTION;
}

void GravityJamRoot::update(float dt)
{
    if(theLevel.isCurrentLevelComplete() && !faulted)
    {
        // run end of level function]
        printf("Level %i complete. Preparing new level...", gj_level);
        scheduleOnce(schedule_selector(GravityJamRoot::endLevel), 0.0);
    }
}

void GravityJamRoot::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

