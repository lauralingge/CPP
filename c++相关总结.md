//
//  main.cpp
//  cpptest
//
//  Created by laura bliss on 2018/10/25.
//  Copyright © 2018 laura bliss. All rights reserved.
//
/**
 <1>The differences between cpp and c
 1 c++ 是面向对象的，而c 是面向过程的。
 2 c++ 的多态继承和封装
 3
 4
 5
 6
 7
 8
 9
 10
 */
/**
 <2>Polymorphism 多态
 */

/**
<3>a*
 */


/**
<4>n个台阶的问题
  有N阶台阶，每一步可以走1步台阶或者2步台阶，求出走到第N阶台阶的方法数。
 */
class StepN
{
    
};



/**
 <5>排序算法及其时间复杂度

 */

/**
 <6>常用的 stl

 */

/**
 <7>内存管理机制(有哪种内存，什么特点、怎么管理的、怎么判断内存泄漏、怎么优化)
 c++ 内存管理机制和原理，怎么判断c++代码是否有内存泄露
 内存分为5个区： 堆、栈 、自由存储区、全局/静态存储区、 常量存储区
 栈：函数在执行的时候，局部变量的存储单元在栈上创建，函数执行结束的时候，这些单元格自动被释放：效率高，存储空间有限
 堆：通过malloc 分配的内存块，释放由我们的程序控制释放，free与对应，如果程序员没有释放，程序运行结束的时候，系统自动回收
 自由存储区：new 等分配的内存块，和堆十分相似，用delete来释放new 申请的内存
 全局/静态存储区：全局变量和静态变量被分配到这一内存块中
 常量存储区：里面存放常量，不允许修改
 
 cocos2dx的内存管理机制和原理、怎么判断cocos2dx代码是否有内存泄露
 内存管理：release autorelease
 cocos2d 所有的类都继承与Ref; Ref的基本原理就是引用计数_referenceCount,当_referenceCount 为0 的时候释放。引用计数通过retain 和release来进行操作
 用label举例，我们在创建一个label，
 
 auto label = Label::create();
 label->setString("KaMi");
 addChild(label);
 
 Label* Label::create(){
 auto ret = new (std::nothrow) Label();
 if (ret){
 ret->autorelease();
 }
 return ret;
 }
 
 在label被create的地方，create函数是一个工厂方法，cocos中很多类都实现了这个方法，其中可以看到ret->autorelease();，这个函数就是把当前对象加入到自动释放池内，对于自动释放池下面会详细讲解。
 （注：Ref初始化的时候引用计数为1不是0）
 
 接下来看一下addChild(label)里面，这个方法是属于Node的，里面跳转的代码有些长，我就截取了一小部分。
 void Node::addChild(Node *child){
 CCASSERT( child != nullptr, "Argument must be non-nil");
 this->addChild(child, child->_localZOrder, child->_name);
 }
 
 void Node::addChild(Node* child, int localZOrder, const std::string &name){
 ...
 addChildHelper(child, localZOrder, INVALID_TAG, name, false);
 }
 
 void Node::addChildHelper(Node* child, int localZOrder, int tag, const std::string &name, bool setTag){
 ...
 this->insertChild(child, localZOrder);
 ...
 }
 
 void Node::insertChild(Node* child, int z){
 _transformUpdated = true;
 _reorderChildDirty = true;
 _children.pushBack(child);
 child->_localZOrder = z;
 }
 
 最终跳转到了insertChild中，通过_children.pushBack(child)把label加入到_children中去。那么问题来了，到底在哪里对引用计数+1操作了呢？答案就在pushBack中，_children是cocos为Ref量身定制的向量Vector<T>，这个向量只能给继承了Ref的类来使用。
 
 void pushBack(T object){
 ...
 _data.push_back( object );
 object->retain();
 }
 
 代码中可以看到object->retain()，对添加进来的对象引用+1操作。那么什么时候-1呢？
 当我们场景移除的时候，应该是要释放在场景中的label的。Node被移除时会调用当前Node的父亲的removeChild函数，此函数最后会调用Node的cleanup函数，cleanup函数是递归函数，会遍历所有的子节点。当cleanup完之后会从父节点的_children这个向量中删除，此时就会调用release函数。
 
 //当某个儿子节点cleanup完之后会调用_children.earse(childIndex)
 iterator erase(ssize_t index){
 ...
 auto it = std::next( begin(), index );
 (*it)->release();
 return _data.erase(it);
 }
 
 release函数就是对当前实例的引用计数-1，如果-1后为0那么释放内存。
 void Ref::release(){
 ...
 --_referenceCount;
 
 if (_referenceCount == 0){
 ...
 delete this;
 }
 }
 自动释放池
 Ref中有autorelease函数，咋一看感觉内存不需要我来管了，他会自动释放。然而这个自动和我脑子里想的自动相差一个孙悟空的跟头，毕竟是c++不是java。先看看autorelease的源码
 Ref* Ref::autorelease()
 {
 PoolManager::getInstance()->getCurrentPool()->addObject(this);
 return this;
 }
 
 代码很短，autorelease并没有release，而是把对象加入到了对象池中。那么这个对象池是什么时候去release里面的对象的呢？接下来就要看Director的mainLoop函数了，这个函数在Director中没有实现，是在DisplayLinkDirector中实现的。
 void DisplayLinkDirector::mainLoop(){
 if (_purgeDirectorInNextLoop){
 ...
 }
 else if (_restartDirectorInNextLoop){
 ...
 }
 else if (! _invalid){
 drawScene();
 
 // release the objects
 PoolManager::getInstance()->getCurrentPool()->clear();
 }
 }
 mainLoop是每一帧调用的函数。我们发现cocos在每一帧结束绘制drawScene之后都会调用PoolManager::getInstance()->getCurrentPool()->clear();的操作。接下去看看clear的实现细节
 
 void AutoreleasePool::clear()
 {
 #if defined(COCOS2D_DEBUG) && (COCOS2D_DEBUG > 0)
 _isClearing = true;
 #endif
 std::vector<Ref*> releasings;
 releasings.swap(_managedObjectArray);
 for (const auto &obj : releasings)
 {
 obj->release();
 }
 #if defined(COCOS2D_DEBUG) && (COCOS2D_DEBUG > 0)
 _isClearing = false;
 #endif
 }
 
 我们发现，在clean里面对所有在_managedObjectArray中的对象都进行了一次release操作，并把它从_managedObjectArray中删除掉。_managedObjectArray是什么，查看前一段代码中addObject的实现细节就知道，autorelease就是把当前对象加入到_managedObjectArray中。
 也就是说，我们创建的Label的时候引用计数为1，然后调用autorelease添加到_managedObjectArray中，之后又被addChild到屏幕中，此时引用计数为2。当一帧绘制结束的时候会系统会调用释放池的clear函数，此函数会遍历所有在自动释放池内的对象并release，最后从对象池中删除之（所以第二帧结束后不会被再次调用release了），此时引用计数为1。当我们把当前场景移除的时候会调用release把引用计数减少至0，并从内存中释放。
 以上就是cocos的的自动内存管理机制
 
 */

/**
 <8>构造函数
 拷贝构造函数

*/

/**
 <9>虚函数
 */

/**
 <10>析构函数

 */
 
/**
<11> 常用的数据结构 数组 链表 树 二叉树 图 字典
 */

/**
<12>二分法 kmp算法
 */

/**
 <13>指针 引用
 */

/**
 
<14>进程线程协程
 */

/**
<15>堆栈
 */

/**
<16>tcp/ip http/https socket
 */

/**
<17>protocalbuff
 */

/**
<18> C++11
    1 CC_CALLBACK_0 CC_CALLBACK_1 CC_CALLBACK_2 CC_CALLBACK_3 的问题
    函数回调的问题，0，1，2，3 代表，后面参数是函数的第几个参数开始的参数

 eg：
 boy->runAction(CCSequence::create(MoveBy::create(1.0f,Point(0,100)),
 CallFunc::create(CC_CALLBACK_0(HelloWorld::callback1,this)),
 NULL));
 void HelloWorld::callback1()
 {
 CCLOG("in callback1");
 //girl1运动，最后回调到callback2
 girl_1->runAction(CCSequence::create(MoveBy::create(1.0f,Point(0,150)),
 CallFunc::create(CC_CALLBACK_0(HelloWorld::callback2,this,girl_1)),
 NULL));
 } 0代表的是this后的参数是 callback2 的第一个参数开始的参数
 void HelloWorld::callback2(Node* sender)
 {
 //girl2运动，最后回调到callback3
 girl_2->runAction(CCSequence::create(MoveBy::create(1.0f,Point(0,200)),
 CallFunc::create(CC_CALLBACK_0(HelloWorld::callback3,this,girl_2,99)),
 NULL));
 
 CCLOG("in callback2,sender tag is:%d",(Sprite*)sender->getTag());
 }
 void HelloWorld::callback3(Node* sender, long data)
 {
 //最终输出
 CCLOG("in callback3,everything is OK,sender tag is:%d,date is:%ld",(Sprite*)sender->getTag(),data);
 CCLOG("girl2 dandan ask:what fake the CC_CALLBACK is?");
 }
 */

/**
 <19>

 */
/**
 <20>
 */