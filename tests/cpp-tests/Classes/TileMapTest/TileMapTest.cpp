#include "TileMapTest.h"
#include "../testResource.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCCustomCommand.h"

enum 
{
    kTagTileMap = 1,
};

Layer* nextTileMapAction();
Layer* backTileMapAction();
Layer* restartTileMapAction();

//------------------------------------------------------------------
//
// TileDemo
//
//------------------------------------------------------------------

enum
{
    IDC_NEXT = 100,
    IDC_BACK,
    IDC_RESTART
};

static int sceneIdx = -1;

#define MAX_LAYER    29

static std::function<Layer*()> createFunctions[] = {
    CLN(TMXIsoZorder),
    CLN(TMXOrthoZorder),
    CLN(TMXIsoVertexZ),
    CLN(TMXOrthoVertexZ),
    CLN(TMXOrthoTest),
    CLN(TMXOrthoTest2),
    CLN(TMXOrthoTest3),
    CLN(TMXOrthoTest4),
    CLN(TMXIsoTest),
    CLN(TMXIsoTest1),
    CLN(TMXIsoTest2),
    CLN(TMXUncompressedTest),
    CLN(TMXHexTest),
    CLN(TMXReadWriteTest),
    CLN(TMXTilesetTest),
    CLN(TMXOrthoObjectsTest),
    CLN(TMXIsoObjectsTest),
    CLN(TMXResizeTest),
    CLN(TMXIsoMoveLayer),
    CLN(TMXOrthoMoveLayer),
    CLN(TMXOrthoFlipTest),
    CLN(TMXOrthoFlipRunTimeTest),
    CLN(TMXOrthoFromXMLTest),
    CLN(TMXOrthoXMLFormatTest),
    CLN(TileMapTest),
    CLN(TileMapEditTest),
    CLN(TMXBug987),
    CLN(TMXBug787),
    CLN(TMXGIDObjectsTest),

};

Layer* createTileMalayer(int nIndex)
{
    return createFunctions[nIndex]();
}

Layer* nextTileMapAction()
{
    sceneIdx++;
    sceneIdx = sceneIdx % MAX_LAYER;

    return createTileMalayer(sceneIdx);
}

Layer* backTileMapAction()
{
    sceneIdx--;
    int total = MAX_LAYER;
    if( sceneIdx < 0 )
        sceneIdx += total;

    return createTileMalayer(sceneIdx);
}

Layer* restartTileMapAction()
{
    return createTileMalayer(sceneIdx);
}


TileDemo::TileDemo(void)
: BaseTest()
{
    auto listener = EventListenerTouchAllAtOnce::create();
    listener->onTouchesMoved = CC_CALLBACK_2(TileDemo::onTouchesMoved, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

TileDemo::~TileDemo(void)
{
}

std::string TileDemo::title() const
{
    return "No title";
}

std::string TileDemo::subtitle() const
{
    return "drag the screen";
}

void TileDemo::onEnter()
{
    BaseTest::onEnter();
}

void TileDemo::onExit()
{
    BaseTest::onExit();
    Director::getInstance()->setDepthTest(false);
}
void TileDemo::restartCallback(Ref* sender)
{
    auto s = new TileMapTestScene();
    s->addChild(restartTileMapAction());

    Director::getInstance()->replaceScene(s);
    s->release();
}

void TileDemo::nextCallback(Ref* sender)
{
    auto s = new TileMapTestScene();
    s->addChild( nextTileMapAction() );
    Director::getInstance()->replaceScene(s);
    s->release();
}

void TileDemo::backCallback(Ref* sender)
{
    auto s = new TileMapTestScene();
    s->addChild( backTileMapAction() );
    Director::getInstance()->replaceScene(s);
    s->release();
}

void TileDemo::onTouchesMoved(const std::vector<Touch*>& touches, Event  *event)
{
    auto touch = touches[0];

    auto diff = touch->getDelta();
    auto node = getChildByTag(kTagTileMap);
    auto currentPos = node->getPosition();
    node->setPosition(currentPos + diff);
}

void TileMapTestScene::runThisTest()
{
    auto layer = nextTileMapAction();
    addChild(layer);

    // fix bug #486, #419.
    // "test" is the default value in Director::setGLDefaultValues()
    // but TransitionTest may setDepthTest(false), we should revert it here
    Director::getInstance()->setDepthTest(true);

    Director::getInstance()->replaceScene(this);
}


//------------------------------------------------------------------
//
// TileMapTest
//
//------------------------------------------------------------------
TileMapTest::TileMapTest()
{
    auto map = TileMapAtlas::create(s_TilesPng,  s_LevelMapTga, 16, 16);
    // Convert it to "alias" (GL_LINEAR filtering)
    map->getTexture()->setAntiAliasTexParameters();
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);

    // If you are not going to use the Map, you can free it now
    // NEW since v0.7
    map->releaseMap();
    
    addChild(map, 0, kTagTileMap);
    
    map->setAnchorPoint( Point(0, 0.5f) );

    auto scale = ScaleBy::create(4, 0.8f);
    auto scaleBack = scale->reverse();

    auto seq = Sequence::create(scale, scaleBack, NULL);

    map->runAction(RepeatForever::create(seq));
}

std::string TileMapTest::title() const
{
    return "TileMapAtlas";
}

//------------------------------------------------------------------
//
// TileMapEditTest
//
//------------------------------------------------------------------
TileMapEditTest::TileMapEditTest()
{
    auto map = TileMapAtlas::create(s_TilesPng, s_LevelMapTga, 16, 16);
    // Create an Aliased Atlas
    map->getTexture()->setAliasTexParameters();
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    // If you are not going to use the Map, you can free it now
    // [tilemap releaseMap);
    // And if you are going to use, it you can access the data with:
    schedule(schedule_selector(TileMapEditTest::updateMap), 0.2f);
    
    addChild(map, 0, kTagTileMap);
    
    map->setAnchorPoint( Point(0, 0) );
    map->setPosition( Point(-20,-200) );
}

void TileMapEditTest::updateMap(float dt)
{
    // IMPORTANT
    //   The only limitation is that you cannot change an empty, or assign an empty tile to a tile
    //   The value 0 not rendered so don't assign or change a tile with value 0

    auto tilemap = (TileMapAtlas*) getChildByTag(kTagTileMap);
    
    //
    // For example you can iterate over all the tiles
    // using this code, but try to avoid the iteration
    // over all your tiles in every frame. It's very expensive
    //    for(int x=0; x < tilemap.tgaInfo->width; x++) {
    //        for(int y=0; y < tilemap.tgaInfo->height; y++) {
    //            Color3B c =[tilemap tileAt:Size(x,y));
    //            if( c.r != 0 ) {
    //                ////----CCLOG("%d,%d = %d", x,y,c.r);
    //            }
    //        }
    //    }
    
    // NEW since v0.7
    Color3B c = tilemap->getTileAt(Point(13,21));        
    c.r++;
    c.r %= 50;
    if( c.r==0)
        c.r=1;
    
    // NEW since v0.7
    tilemap->setTile(c, Point(13,21) );             
}

std::string TileMapEditTest::title() const
{
    return "Editable TileMapAtlas";
}

//------------------------------------------------------------------
//
// TMXOrthoTest
//
//------------------------------------------------------------------
TMXOrthoTest::TMXOrthoTest()
{
    //
    // Test orthogonal with 3d camera and anti-alias textures
    //
    // it should not flicker. No artifacts should appear
    //
    //auto color = LayerColor::create( Color4B(64,64,64,255) );
    //addChild(color, -1);

    auto map = TMXTiledMap::create("TileMaps/orthogonal-test2.tmx");

    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);

    auto scale = ScaleBy::create(10, 0.1f);
    auto back = scale->reverse();
    auto seq = Sequence::create(scale, back, NULL);
    auto repeat = RepeatForever::create(seq);
    map->runAction(repeat);

//    float x, y, z;
//    map->getCamera()->getEye(&x, &y, &z);
//    map->getCamera()->setEye(x-200, y, z+300);    
}

void TMXOrthoTest::onEnter()
{
    TileDemo::onEnter();

    Director::getInstance()->setProjection(Director::Projection::_3D);
}

void TMXOrthoTest::onExit()
{
    Director::getInstance()->setProjection(Director::Projection::DEFAULT);
    TileDemo::onExit();
}

std::string TMXOrthoTest::title() const
{
    return "TMX Orthogonal test";
}

//------------------------------------------------------------------
//
// TMXOrthoTest2
//
//------------------------------------------------------------------
TMXOrthoTest2::TMXOrthoTest2()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test1.tmx");
    addChild(map, 0, kTagTileMap);

    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);

    auto& children = map->getChildren();
    SpriteBatchNode* child = NULL;

    for(const auto &obj : children) {
        child = static_cast<SpriteBatchNode*>(obj);
        child->getTexture()->setAntiAliasTexParameters();
    }

    map->runAction( ScaleBy::create(2, 0.5f) ) ;
}

std::string TMXOrthoTest2::title() const
{
    return "TMX Ortho test2";
}

//------------------------------------------------------------------
//
// TMXOrthoTest3
//
//------------------------------------------------------------------
TMXOrthoTest3::TMXOrthoTest3()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test3.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    auto& children = map->getChildren();
    SpriteBatchNode* child = NULL;

    for(const auto &node : children) {
        child = static_cast<SpriteBatchNode*>(node);
        child->getTexture()->setAntiAliasTexParameters();
    }
    
    map->setScale(0.2f);
    map->setAnchorPoint( Point(0.5f, 0.5f) );
}

std::string TMXOrthoTest3::title() const
{
    return "TMX anchorPoint test";
}

//------------------------------------------------------------------
//
// TMXOrthoTest4
//
//------------------------------------------------------------------
TMXOrthoTest4::TMXOrthoTest4()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test4.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s1 = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s1.width,s1.height);

    SpriteBatchNode* child = nullptr;
    
    auto& children = map->getChildren();
    
    for(const auto &node : children) {
        child = static_cast<SpriteBatchNode*>(node);
        child->getTexture()->setAntiAliasTexParameters();
    }
    
    map->setAnchorPoint(Point(0, 0));

    auto layer = map->getLayer("Layer 0");
    auto s = layer->getLayerSize();
    
    Sprite* sprite;
    sprite = layer->getTileAt(Point(0,0));
    sprite->setScale(2);
    sprite = layer->getTileAt(Point(s.width-1,0));
    sprite->setScale(2);
    sprite = layer->getTileAt(Point(0,s.height-1));
    sprite->setScale(2);
    sprite = layer->getTileAt(Point(s.width-1,s.height-1));
    sprite->setScale(2);

    schedule( schedule_selector(TMXOrthoTest4::removeSprite), 2 );

}

void TMXOrthoTest4::removeSprite(float dt)
{
    unschedule(schedule_selector(TMXOrthoTest4::removeSprite));

    auto map = static_cast<TMXTiledMap*>( getChildByTag(kTagTileMap) );
    auto layer = map->getLayer("Layer 0");
    auto s = layer->getLayerSize();

    auto sprite = layer->getTileAt( Point(s.width-1,0) );
    auto sprite2 = layer->getTileAt(Point(s.width-1, s.height-1));
    layer->removeChild(sprite, true);
    auto sprite3 = layer->getTileAt(Point(2, s.height-1));
    layer->removeChild(sprite3, true);
    layer->removeChild(sprite2, true);
}

std::string TMXOrthoTest4::title() const
{
    return "TMX width/height test";
}

//------------------------------------------------------------------
//
// TMXReadWriteTest
//
//------------------------------------------------------------------
enum
{
    SID_UPDATECOL = 100,
    SID_REPAINTWITHGID,
    SID_REMOVETILES
};

TMXReadWriteTest::TMXReadWriteTest()
{
    _gid = 0;
    
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test2.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);

    
    auto layer = map->getLayer("Layer 0");
    layer->getTexture()->setAntiAliasTexParameters();

    map->setScale( 1 );

    auto tile0 = layer->getTileAt(Point(1,63));
    auto tile1 = layer->getTileAt(Point(2,63));
    auto tile2 = layer->getTileAt(Point(3,62));//Point(1,62));
    auto tile3 = layer->getTileAt(Point(2,62));
    tile0->setAnchorPoint( Point(0.5f, 0.5f) );
    tile1->setAnchorPoint( Point(0.5f, 0.5f) );
    tile2->setAnchorPoint( Point(0.5f, 0.5f) );
    tile3->setAnchorPoint( Point(0.5f, 0.5f) );

    auto move = MoveBy::create(0.5f, Point(0,160));
    auto rotate = RotateBy::create(2, 360);
    auto scale = ScaleBy::create(2, 5);
    auto opacity = FadeOut::create(2);
    auto fadein = FadeIn::create(2);
    auto scaleback = ScaleTo::create(1, 1);
    auto finish = CallFuncN::create(CC_CALLBACK_1(TMXReadWriteTest::removeSprite, this));
    auto seq0 = Sequence::create(move, rotate, scale, opacity, fadein, scaleback, finish, NULL);
    auto seq1 = seq0->clone();
    auto seq2 = seq0->clone();
    auto seq3 = seq0->clone();
    
    tile0->runAction(seq0);
    tile1->runAction(seq1);
    tile2->runAction(seq2);
    tile3->runAction(seq3);
    
    
    _gid = layer->getTileGIDAt(Point(0,63));
    ////----CCLOG("Tile GID at:(0,63) is: %d", _gid);

    schedule(schedule_selector(TMXReadWriteTest::updateCol), 2.0f); 
    schedule(schedule_selector(TMXReadWriteTest::repaintWithGID), 2.05f);
    schedule(schedule_selector(TMXReadWriteTest::removeTiles), 1.0f); 

    ////----CCLOG("++++atlas quantity: %d", layer->textureAtlas()->getTotalQuads());
    ////----CCLOG("++++children: %d", layer->getChildren()->count() );
    
    _gid2 = 0;
}

void TMXReadWriteTest::removeSprite(Node* sender)
{
    ////----CCLOG("removing tile: %x", sender);
    auto p = ((Node*)sender)->getParent();

    if (p)
    {
        p->removeChild((Node*)sender, true);
    }    
    
    //////----CCLOG("atlas quantity: %d", p->textureAtlas()->totalQuads());
}

void TMXReadWriteTest::updateCol(float dt)
{    
    auto map = (TMXTiledMap*)getChildByTag(kTagTileMap);
    auto layer = (TMXLayer*)map->getChildByTag(0);

    ////----CCLOG("++++atlas quantity: %d", layer->textureAtlas()->getTotalQuads());
    ////----CCLOG("++++children: %d", layer->getChildren()->count() );


    auto s = layer->getLayerSize();

    for( int y=0; y< s.height; y++ ) 
    {
        layer->setTileGID(_gid2, Point((float)3, (float)y));
    }
    
    _gid2 = (_gid2 + 1) % 80;
}

void TMXReadWriteTest::repaintWithGID(float dt)
{
//    unschedule:_cmd);
    
    auto map = (TMXTiledMap*)getChildByTag(kTagTileMap);
    auto layer = (TMXLayer*)map->getChildByTag(0);
    
    auto s = layer->getLayerSize();
    for( int x=0; x<s.width;x++) 
    {
        int y = (int)s.height-1;
        unsigned int tmpgid = layer->getTileGIDAt( Point((float)x, (float)y) );
        layer->setTileGID(tmpgid+1, Point((float)x, (float)y));
    }
}

void TMXReadWriteTest::removeTiles(float dt)
{
    unschedule(schedule_selector(TMXReadWriteTest::removeTiles));

    auto map = (TMXTiledMap*)getChildByTag(kTagTileMap);
    auto layer = (TMXLayer*)map->getChildByTag(0);
    auto s = layer->getLayerSize();

    for( int y=0; y< s.height; y++ ) 
    {
        layer->removeTileAt( Point(5.0, (float)y) );
    }
}



std::string TMXReadWriteTest::title() const
{
    return "TMX Read/Write test";
}

//------------------------------------------------------------------
//
// TMXHexTest
//
//------------------------------------------------------------------
TMXHexTest::TMXHexTest()
{
    auto color = LayerColor::create( Color4B(64,64,64,255) );
    addChild(color, -1);
    
    auto map = TMXTiledMap::create("TileMaps/hexa-test.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
}

std::string TMXHexTest::title() const
{
    return "TMX Hex tes";
}

//------------------------------------------------------------------
//
// TMXIsoTest
//
//------------------------------------------------------------------
TMXIsoTest::TMXIsoTest()
{
    auto color = LayerColor::create( Color4B(64,64,64,255) );
    addChild(color, -1);
    
    auto map = TMXTiledMap::create("TileMaps/iso-test.tmx");
    addChild(map, 0, kTagTileMap);        
    
    // move map to the center of the screen
    auto ms = map->getMapSize();
    auto ts = map->getTileSize();
    map->runAction( MoveTo::create(1.0f, Point( -ms.width * ts.width/2, -ms.height * ts.height/2 )) ); 
}

std::string TMXIsoTest::title() const
{
    return "TMX Isometric test 0";
}

//------------------------------------------------------------------
//
// TMXIsoTest1
//
//------------------------------------------------------------------
TMXIsoTest1::TMXIsoTest1()
{
    auto color = LayerColor::create( Color4B(64,64,64,255) );
    addChild(color, -1);
    
    auto map = TMXTiledMap::create("TileMaps/iso-test1.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    map->setAnchorPoint(Point(0.5f, 0.5f));
}

std::string TMXIsoTest1::title() const
{
    return "TMX Isometric test + anchorPoint";
}

//------------------------------------------------------------------
//
// TMXIsoTest2
//
//------------------------------------------------------------------
TMXIsoTest2::TMXIsoTest2()
{
    auto color = LayerColor::create( Color4B(64,64,64,255) );
    addChild(color, -1);
    
    auto map = TMXTiledMap::create("TileMaps/iso-test2.tmx");
    addChild(map, 0, kTagTileMap);    
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    // move map to the center of the screen
    auto ms = map->getMapSize();
    auto ts = map->getTileSize();
    map->runAction( MoveTo::create(1.0f, Point( -ms.width * ts.width/2, -ms.height * ts.height/2 ) ));
}

std::string TMXIsoTest2::title() const
{
    return "TMX Isometric test 2";
}

//------------------------------------------------------------------
//
// TMXUncompressedTest
//
//------------------------------------------------------------------
TMXUncompressedTest::TMXUncompressedTest()
{
    auto color = LayerColor::create( Color4B(64,64,64,255) );
    addChild(color, -1);
    
    auto map = TMXTiledMap::create("TileMaps/iso-test2-uncompressed.tmx");
    addChild(map, 0, kTagTileMap);    
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    // move map to the center of the screen
    auto ms = map->getMapSize();
    auto ts = map->getTileSize();
    map->runAction(MoveTo::create(1.0f, Point( -ms.width * ts.width/2, -ms.height * ts.height/2 ) ));
    
    // testing release map
    TMXLayer* layer;
    
    auto& children = map->getChildren();
    for(const auto &node : children) {
        layer= static_cast<TMXLayer*>(node);
        layer->releaseMap();
    }

}

std::string TMXUncompressedTest::title() const
{
    return "TMX Uncompressed test";
}

//------------------------------------------------------------------
//
// TMXTilesetTest
//
//------------------------------------------------------------------
TMXTilesetTest::TMXTilesetTest()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test5.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    TMXLayer* layer;
    layer = map->getLayer("Layer 0");
    layer->getTexture()->setAntiAliasTexParameters();
    
    layer = map->getLayer("Layer 1");
    layer->getTexture()->setAntiAliasTexParameters();

    layer = map->getLayer("Layer 2");
    layer->getTexture()->setAntiAliasTexParameters();
}

std::string TMXTilesetTest::title() const
{
    return "TMX Tileset test";
}

//------------------------------------------------------------------
//
// TMXOrthoObjectsTest
//
//------------------------------------------------------------------
TMXOrthoObjectsTest::TMXOrthoObjectsTest()
{
    auto map = TMXTiledMap::create("TileMaps/ortho-objects.tmx");
    addChild(map, -1, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    auto group = map->getObjectGroup("Object Group 1");
    auto& objects = group->getObjects();

    Value objectsVal = Value(objects);
    CCLOG("%s", objectsVal.getDescription().c_str());
}

void TMXOrthoObjectsTest::draw(Renderer *renderer, const kmMat4 &transform, bool transformUpdated)
{
    _renderCmd.init(_globalZOrder);
    _renderCmd.func = CC_CALLBACK_0(TMXOrthoObjectsTest::onDraw, this, transform, transformUpdated);
    renderer->addCommand(&_renderCmd);
}

void TMXOrthoObjectsTest::onDraw(const kmMat4 &transform, bool transformUpdated)
{
    kmGLPushMatrix();
    kmGLLoadMatrix(&transform);
    
    auto map = static_cast<TMXTiledMap*>( getChildByTag(kTagTileMap) );
    auto pos = map->getPosition();
    auto group = map->getObjectGroup("Object Group 1");

    auto& objects = group->getObjects();

    for (auto& obj : objects)
    {
        ValueMap& dict = obj.asValueMap();
        
        float x = dict["x"].asFloat();
        float y = dict["y"].asFloat();
        float width = dict["width"].asFloat();
        float height = dict["height"].asFloat();
        
        glLineWidth(3);
        
        DrawPrimitives::drawLine( pos + Point(x, y), pos + Point((x+width), y) );
        DrawPrimitives::drawLine( pos + Point((x+width), y), pos + Point((x+width), (y+height)) );
        DrawPrimitives::drawLine( pos + Point((x+width), (y+height)), pos + Point(x, (y+height)) );
        DrawPrimitives::drawLine( pos + Point(x, (y+height)), pos + Point(x, y) );
        
        glLineWidth(1);
    }
    
    kmGLPopMatrix();
}

std::string TMXOrthoObjectsTest::title() const
{
    return "TMX Ortho object test";
}

std::string TMXOrthoObjectsTest::subtitle() const
{
    return "You should see a white box around the 3 platforms";
}


//------------------------------------------------------------------
//
// TMXIsoObjectsTest
//
//------------------------------------------------------------------

TMXIsoObjectsTest::TMXIsoObjectsTest()
{
    auto map = TMXTiledMap::create("TileMaps/iso-test-objectgroup.tmx");
    addChild(map, -1, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);

    auto group = map->getObjectGroup("Object Group 1");

    auto& objects = group->getObjects();
    
    Value objectsVal = Value(objects);
    CCLOG("%s", objectsVal.getDescription().c_str());
}

void TMXIsoObjectsTest::draw(Renderer *renderer, const kmMat4 &transform, bool transformUpdated)
{
    _renderCmd.init(_globalZOrder);
    _renderCmd.func = CC_CALLBACK_0(TMXIsoObjectsTest::onDraw, this, transform, transformUpdated);
    renderer->addCommand(&_renderCmd);
}

void TMXIsoObjectsTest::onDraw(const kmMat4 &transform, bool transformUpdated)
{
    kmGLPushMatrix();
    kmGLLoadMatrix(&transform);

    auto map = (TMXTiledMap*) getChildByTag(kTagTileMap);
    auto pos = map->getPosition();
    auto group = map->getObjectGroup("Object Group 1");

    auto& objects = group->getObjects();
    for (auto& obj : objects)
    {
        ValueMap& dict = obj.asValueMap();
        float x = dict["x"].asFloat();
        float y = dict["y"].asFloat();
        float width = dict["width"].asFloat();
        float height = dict["height"].asFloat();
        
        glLineWidth(3);
        
        DrawPrimitives::drawLine( pos + Point(x,y), pos + Point(x+width,y) );
        DrawPrimitives::drawLine( pos + Point(x+width,y), pos + Point(x+width,y+height) );
        DrawPrimitives::drawLine( pos + Point(x+width,y+height), pos + Point(x,y+height) );
        DrawPrimitives::drawLine( pos + Point(x,y+height), pos + Point(x,y) );
        
        glLineWidth(1);
    }

    kmGLPopMatrix();
}

std::string TMXIsoObjectsTest::title() const
{
    return "TMX Iso object test";
}

std::string TMXIsoObjectsTest::subtitle() const
{
    return "You need to parse them manually. See bug #810";
}


//------------------------------------------------------------------
//
// TMXResizeTest
//
//------------------------------------------------------------------

TMXResizeTest::TMXResizeTest()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test5.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);

    TMXLayer* layer;
    layer = map->getLayer("Layer 0");

    auto ls = layer->getLayerSize();
    for (unsigned int y = 0; y < ls.height; y++) 
    {
        for (unsigned int x = 0; x < ls.width; x++) 
        {
            layer->setTileGID(1, Point( x, y ) );
        }
    }        
}

std::string TMXResizeTest::title() const
{
    return "TMX resize test";
}

std::string TMXResizeTest::subtitle() const
{
    return "Should not crash. Testing issue #740";
}


//------------------------------------------------------------------
//
// TMXIsoZorder
//
//------------------------------------------------------------------
TMXIsoZorder::TMXIsoZorder()
{
    auto map = TMXTiledMap::create("TileMaps/iso-test-zorder.tmx");
    addChild(map, 0, kTagTileMap);

    auto s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    map->setPosition(Point(-s.width/2,0));
    
    _tamara = Sprite::create(s_pathSister1);
    map->addChild(_tamara, (int)map->getChildren().size() );
    _tamara->retain();
    int mapWidth = map->getMapSize().width * map->getTileSize().width;
    _tamara->setPosition(CC_POINT_PIXELS_TO_POINTS(Point( mapWidth/2,0)));
    _tamara->setAnchorPoint(Point(0.5f,0));

    
    auto move = MoveBy::create(10, Point(300,250));
    auto back = move->reverse();
    auto seq = Sequence::create(move, back,NULL);
    _tamara->runAction( RepeatForever::create(seq) );
    
    schedule( schedule_selector(TMXIsoZorder::repositionSprite) );
}

TMXIsoZorder::~TMXIsoZorder()
{
    _tamara->release();
}

void TMXIsoZorder::onExit()
{
    unschedule(schedule_selector(TMXIsoZorder::repositionSprite));
    TileDemo::onExit();
}

void TMXIsoZorder::repositionSprite(float dt)
{
    auto p = _tamara->getPosition();
    p = CC_POINT_POINTS_TO_PIXELS(p);
    auto map = getChildByTag(kTagTileMap);
    
    // there are only 4 layers. (grass and 3 trees layers)
    // if tamara < 48, z=4
    // if tamara < 96, z=3
    // if tamara < 144,z=2
    
    int newZ = 4 - (p.y / 48);
    newZ = std::max(newZ,0);
    
    map->reorderChild(_tamara, newZ);    
}

std::string TMXIsoZorder::title() const
{
    return "TMX Iso Zorder";
}

std::string TMXIsoZorder::subtitle() const
{
    return "Sprite should hide behind the trees";
}


//------------------------------------------------------------------
//
// TMXOrthoZorder
//
//------------------------------------------------------------------
TMXOrthoZorder::TMXOrthoZorder()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test-zorder.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    _tamara = Sprite::create(s_pathSister1);
    map->addChild(_tamara,  (int)map->getChildren().size());
    _tamara->retain();
    _tamara->setAnchorPoint(Point(0.5f,0));

    
    auto move = MoveBy::create(10, Point(400,450));
    auto back = move->reverse();
    auto seq = Sequence::create(move, back,NULL);
    _tamara->runAction( RepeatForever::create(seq));
    
    schedule( schedule_selector(TMXOrthoZorder::repositionSprite));
}

TMXOrthoZorder::~TMXOrthoZorder()
{
    _tamara->release();
}

void TMXOrthoZorder::repositionSprite(float dt)
{
    auto p = _tamara->getPosition();
    p = CC_POINT_POINTS_TO_PIXELS(p);
    auto map = getChildByTag(kTagTileMap);
    
    // there are only 4 layers. (grass and 3 trees layers)
    // if tamara < 81, z=4
    // if tamara < 162, z=3
    // if tamara < 243,z=2

    // -10: customization for this particular sample
    int newZ = 4 - ( (p.y-10) / 81);
    newZ = std::max(newZ,0);

    map->reorderChild(_tamara, newZ);
}

std::string TMXOrthoZorder::title() const
{
    return "TMX Ortho Zorder";
}

std::string TMXOrthoZorder::subtitle() const
{
    return "Sprite should hide behind the trees";
}


//------------------------------------------------------------------
//
// TMXIsoVertexZ
//
//------------------------------------------------------------------
TMXIsoVertexZ::TMXIsoVertexZ()
{
    auto map = TMXTiledMap::create("TileMaps/iso-test-vertexz.tmx");
    addChild(map, 0, kTagTileMap);
    
    auto s = map->getContentSize();
    map->setPosition( Point(-s.width/2,0) );
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    // because I'm lazy, I'm reusing a tile as an sprite, but since this method uses vertexZ, you
    // can use any Sprite and it will work OK.
    auto layer = map->getLayer("Trees");
    _tamara = layer->getTileAt( Point(29,29) );
    _tamara->retain();
    
    auto move = MoveBy::create(10, Point(300,250) * (1/CC_CONTENT_SCALE_FACTOR()));
    auto back = move->reverse();
    auto seq = Sequence::create(move, back,NULL);
    _tamara->runAction( RepeatForever::create(seq) );
    
    schedule( schedule_selector(TMXIsoVertexZ::repositionSprite));
    
}

TMXIsoVertexZ::~TMXIsoVertexZ()
{
    _tamara->release();
}

void TMXIsoVertexZ::repositionSprite(float dt)
{
    // tile height is 64x32
    // map size: 30x30
    auto p = _tamara->getPosition();
    p = CC_POINT_POINTS_TO_PIXELS(p);
    float newZ = -(p.y+32) /16;
    _tamara->setPositionZ( newZ );
}

void TMXIsoVertexZ::onEnter()
{
    TileDemo::onEnter();
    
    // TIP: 2d projection should be used
    Director::getInstance()->setProjection(Director::Projection::_2D);
    Director::getInstance()->setDepthTest(true);
}

void TMXIsoVertexZ::onExit()
{
    // At exit use any other projection. 
    Director::getInstance()->setProjection(Director::Projection::DEFAULT);
    Director::getInstance()->setDepthTest(false);
    TileDemo::onExit();
}

std::string TMXIsoVertexZ::title() const
{
    return "TMX Iso VertexZ";
}

std::string TMXIsoVertexZ::subtitle() const
{
    return "Sprite should hide behind the trees";
}


//------------------------------------------------------------------
//
// TMXOrthoVertexZ
//
//------------------------------------------------------------------
TMXOrthoVertexZ::TMXOrthoVertexZ()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test-vertexz.tmx");
    addChild(map, 0, kTagTileMap);
    
    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
    
    // because I'm lazy, I'm reusing a tile as an sprite, but since this method uses vertexZ, you
    // can use any Sprite and it will work OK.
    auto layer = map->getLayer("trees");
    _tamara = layer->getTileAt(Point(0,11));
    CCLOG("%p vertexZ: %f", _tamara, _tamara->getPositionZ());
    _tamara->retain();

    auto move = MoveBy::create(10, Point(400,450) * (1/CC_CONTENT_SCALE_FACTOR()));
    auto back = move->reverse();
    auto seq = Sequence::create(move, back,NULL);
    _tamara->runAction( RepeatForever::create(seq));
    
    schedule(schedule_selector(TMXOrthoVertexZ::repositionSprite));
    
}

TMXOrthoVertexZ::~TMXOrthoVertexZ()
{
    _tamara->release();
}

void TMXOrthoVertexZ::repositionSprite(float dt)
{
    // tile height is 101x81
    // map size: 12x12
    auto p = _tamara->getPosition();
    p = CC_POINT_POINTS_TO_PIXELS(p);
    _tamara->setPositionZ( -( (p.y+81) /81) );
}

void TMXOrthoVertexZ::onEnter()
{
    TileDemo::onEnter();
    
    // TIP: 2d projection should be used
    Director::getInstance()->setProjection(Director::Projection::_2D);
    Director::getInstance()->setDepthTest(true);
}

void TMXOrthoVertexZ::onExit()
{
    // At exit use any other projection. 
    Director::getInstance()->setProjection(Director::Projection::DEFAULT);
    Director::getInstance()->setDepthTest(false);
    TileDemo::onExit();
}

std::string TMXOrthoVertexZ::title() const
{
    return "TMX Ortho vertexZ";
}

std::string TMXOrthoVertexZ::subtitle() const
{
    return "Sprite should hide behind the trees";
}


//------------------------------------------------------------------
//
// TMXIsoMoveLayer
//
//------------------------------------------------------------------
TMXIsoMoveLayer::TMXIsoMoveLayer()
{
    auto map = TMXTiledMap::create("TileMaps/iso-test-movelayer.tmx");
    addChild(map, 0, kTagTileMap);
    
    map->setPosition(Point(-700,-50));

    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
}

std::string TMXIsoMoveLayer::title() const
{
    return "TMX Iso Move Layer";
}

std::string TMXIsoMoveLayer::subtitle() const
{
    return "Trees should be horizontally aligned";
}


//------------------------------------------------------------------
//
// TMXOrthoMoveLayer
//
//------------------------------------------------------------------
TMXOrthoMoveLayer::TMXOrthoMoveLayer()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test-movelayer.tmx");
    addChild(map, 0, kTagTileMap);

    Size CC_UNUSED s = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s.width,s.height);
}

std::string TMXOrthoMoveLayer::title() const
{
    return "TMX Ortho Move Layer";
}

std::string TMXOrthoMoveLayer::subtitle() const
{
    return "Trees should be horizontally aligned";
}

//------------------------------------------------------------------
//
// TMXTilePropertyTest
//
//------------------------------------------------------------------

TMXTilePropertyTest::TMXTilePropertyTest()
{
    auto map = TMXTiledMap::create("TileMaps/ortho-tile-property.tmx");
    addChild(map ,0 ,kTagTileMap);

    for(int i=1;i<=20;i++){
        for(const auto& value : map->getPropertiesForGID(i).asValueMap())
        {
            log("GID:%i, Properties:%s, %s", i, value.first.c_str(), value.second.asString().c_str());
        }
    }
}

std::string TMXTilePropertyTest::title() const
{
    return "TMX Tile Property Test";
}

std::string TMXTilePropertyTest::subtitle() const
{
    return "In the console you should see tile properties";
}

//------------------------------------------------------------------
//
// TMXOrthoFlipTest
//
//------------------------------------------------------------------

TMXOrthoFlipTest::TMXOrthoFlipTest()
{
    auto map = TMXTiledMap::create("TileMaps/ortho-rotation-test.tmx");
    addChild(map, 0, kTagTileMap);

    Size CC_UNUSED s = map->getContentSize();
    log("ContentSize: %f, %f", s.width,s.height);

    auto& children = map->getChildren();
    for(const auto &node : children) {
        auto child = static_cast<SpriteBatchNode*>(node);
        child->getTexture()->setAntiAliasTexParameters();
    }

    auto action = ScaleBy::create(2, 0.5f);
    map->runAction(action);
}

std::string TMXOrthoFlipTest::title() const
{
    return "TMX tile flip test";
}

//------------------------------------------------------------------
//
// TMXOrthoFlipRunTimeTest
//
//------------------------------------------------------------------

TMXOrthoFlipRunTimeTest::TMXOrthoFlipRunTimeTest()
{
    auto map = TMXTiledMap::create("TileMaps/ortho-rotation-test.tmx");
    addChild(map, 0, kTagTileMap);

    auto s = map->getContentSize();
    log("ContentSize: %f, %f", s.width,s.height);

    auto& children = map->getChildren();
    for(const auto &node : children) {
        auto child = static_cast<SpriteBatchNode*>(node);
        child->getTexture()->setAntiAliasTexParameters();
    }

    auto action = ScaleBy::create(2, 0.5f);
    map->runAction(action);

    schedule(schedule_selector(TMXOrthoFlipRunTimeTest::flipIt), 1.0f);
}

std::string TMXOrthoFlipRunTimeTest::title() const
{
    return "TMX tile flip run time test";
}

std::string TMXOrthoFlipRunTimeTest::subtitle() const
{
    return "in 2 sec bottom left tiles will flip";
}

void TMXOrthoFlipRunTimeTest::flipIt(float dt)
{
    auto map = (TMXTiledMap*) getChildByTag(kTagTileMap); 
    auto layer = map->getLayer("Layer 0");

    //blue diamond 
    auto tileCoord = Point(1,10);
    int flags;
    unsigned int GID = layer->getTileGIDAt(tileCoord, (TMXTileFlags*)&flags);
    // Vertical
    if( flags & kTMXTileVerticalFlag )
        flags &= ~kTMXTileVerticalFlag;
    else
        flags |= kTMXTileVerticalFlag;
    layer->setTileGID(GID ,tileCoord, (TMXTileFlags)flags);


    tileCoord = Point(1,8);    
    GID = layer->getTileGIDAt(tileCoord, (TMXTileFlags*)&flags);
    // Vertical
    if( flags & kTMXTileVerticalFlag )
        flags &= ~kTMXTileVerticalFlag;
    else
        flags |= kTMXTileVerticalFlag;    
    layer->setTileGID(GID ,tileCoord, (TMXTileFlags)flags);


    tileCoord = Point(2,8);
    GID = layer->getTileGIDAt(tileCoord, (TMXTileFlags*)&flags);
    // Horizontal
    if( flags & kTMXTileHorizontalFlag )
        flags &= ~kTMXTileHorizontalFlag;
    else
        flags |= kTMXTileHorizontalFlag;    
    layer->setTileGID(GID, tileCoord, (TMXTileFlags)flags);    
}
//------------------------------------------------------------------
//
// TMXOrthoFromXMLTest
//
//------------------------------------------------------------------

TMXOrthoFromXMLTest::TMXOrthoFromXMLTest()
{
    std::string resources = "TileMaps";        // partial paths are OK as resource paths.
    std::string file = resources + "/orthogonal-test1.tmx";

    auto str = String::createWithContentsOfFile(FileUtils::getInstance()->fullPathForFilename(file.c_str()).c_str());
    CCASSERT(str != NULL, "Unable to open file");

    auto map = TMXTiledMap::createWithXML(str->getCString() ,resources.c_str());
    addChild(map, 0, kTagTileMap);

    auto s = map->getContentSize();
    log("ContentSize: %f, %f", s.width,s.height);

    auto& children = map->getChildren();
    for(const auto &node : children) {
        auto child = static_cast<SpriteBatchNode*>(node);
        child->getTexture()->setAntiAliasTexParameters();
    }

    auto action = ScaleBy::create(2, 0.5f);
    map->runAction(action);
}

std::string TMXOrthoFromXMLTest::title() const
{
    return "TMX created from XML test";
}
//------------------------------------------------------------------
//
// TMXOrthoXMLFormatTest
//
//------------------------------------------------------------------

TMXOrthoXMLFormatTest::TMXOrthoXMLFormatTest()
{
    // this test tests for:
    // 1. load xml format tilemap
    // 2. gid lower than firstgid is ignored
    // 3. firstgid in tsx is ignored, tile property in tsx loaded correctly.
    auto map = TMXTiledMap::create("TileMaps/xml-test.tmx");
    addChild(map, 0, kTagTileMap);
    
    auto s = map->getContentSize();
    log("ContentSize: %f, %f", s.width,s.height);
    
    auto& children = map->getChildren();
    for(const auto &node : children) {
        auto child = static_cast<SpriteBatchNode*>(node);
        child->getTexture()->setAntiAliasTexParameters();
    }
    
    for(int i=24;i<=26;i++){
        log("GID:%i, Properties:%s", i, map->getPropertiesForGID(i).asValueMap()["name"].asString().c_str());
    }
    
    auto action = ScaleBy::create(2, 0.5f);
    map->runAction(action);
}

std::string TMXOrthoXMLFormatTest::title() const
{
    return "you should see blue, green and yellow in console.";
}

//------------------------------------------------------------------
//
// TMXBug987
//
//------------------------------------------------------------------
TMXBug987::TMXBug987()
{
    auto map = TMXTiledMap::create("TileMaps/orthogonal-test6.tmx");
    addChild(map, 0, kTagTileMap);

    Size CC_UNUSED s1 = map->getContentSize();
    CCLOG("ContentSize: %f, %f", s1.width,s1.height);

    auto& children = map->getChildren();
    for(const auto &child : children) {
        auto node = static_cast<TMXLayer*>(child);
        node->getTexture()->setAntiAliasTexParameters();
    }

    map->setAnchorPoint(Point(0, 0));
    auto layer = map->getLayer("Tile Layer 1");
    layer->setTileGID(3, Point(2,2));
}

std::string TMXBug987::title() const
{
    return "TMX Bug 987";
}

std::string TMXBug987::subtitle() const
{
    return "You should see an square";
}

//------------------------------------------------------------------
//
// TMXBug787
//
//------------------------------------------------------------------
TMXBug787::TMXBug787()
{
    auto map = TMXTiledMap::create("TileMaps/iso-test-bug787.tmx");
    addChild(map, 0, kTagTileMap);

    map->setScale(0.25f);
}

std::string TMXBug787::title() const
{
    return "TMX Bug 787";
}

std::string TMXBug787::subtitle() const
{
    return "You should see a map";
}

//------------------------------------------------------------------
//
// TMXGIDObjectsTest
//
//------------------------------------------------------------------
TMXGIDObjectsTest::TMXGIDObjectsTest()
{
    auto map = TMXTiledMap::create("TileMaps/test-object-layer.tmx");
    addChild(map, -1, kTagTileMap);

    Size CC_UNUSED s = map->getContentSize();
    CCLOG("Contentsize: %f, %f", s.width, s.height);

    CCLOG("----> Iterating over all the group objets");
    //auto group = map->objectGroupNamed("Object Layer 1");

}

void TMXGIDObjectsTest::draw(Renderer *renderer, const kmMat4 &transform, bool transformUpdated)
{
    _renderCmd.init(_globalZOrder);
    _renderCmd.func = CC_CALLBACK_0(TMXGIDObjectsTest::onDraw, this, transform, transformUpdated);
    renderer->addCommand(&_renderCmd);
}

void TMXGIDObjectsTest::onDraw(const kmMat4 &transform, bool transformUpdated)
{
    kmGLPushMatrix();
    kmGLLoadMatrix(&transform);

    auto map = (TMXTiledMap*)getChildByTag(kTagTileMap);
    auto pos = map->getPosition();
    auto group = map->getObjectGroup("Object Layer 1");
    
    auto& objects = group->getObjects();
    for (auto& obj : objects)
    {
        ValueMap& dict = obj.asValueMap();
        
        float x = dict["x"].asFloat();
        float y = dict["y"].asFloat();
        float width = dict["width"].asFloat();
        float height = dict["height"].asFloat();
        
        glLineWidth(3);
        
        DrawPrimitives::drawLine(pos + Point(x, y), pos + Point(x + width, y));
        DrawPrimitives::drawLine(pos + Point(x + width, y), pos + Point(x + width, y + height));
        DrawPrimitives::drawLine(pos + Point(x + width,y + height), pos + Point(x,y + height));
        DrawPrimitives::drawLine(pos + Point(x,y + height), pos + Point(x,y));
        
        glLineWidth(1);
    }
    
    kmGLPopMatrix();
}

std::string TMXGIDObjectsTest::title() const
{
    return "TMX GID objects";
}

std::string TMXGIDObjectsTest::subtitle() const
{
    return "Tiles are created from an object group";
}
