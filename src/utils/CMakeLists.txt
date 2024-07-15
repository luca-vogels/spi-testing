set(TESTING_SRC
  CallbackQueueLock.hpp
  CallbackQueueNaive.hpp
  CallbackQueueRecycle.hpp
  CallbackQueueTwoParty.hpp
  FlowRepresentation.hpp
  FlowRepresentation.cpp
  Future.hpp
  Future.cpp
  HardwareUtils.hpp
  QueueAtomic.hpp
  QueueLock.hpp
  QueueMoodyCamel.hpp
  QueueRing.hpp
  QueueTwoPartyAtomic.hpp
  QueueTwoPartyHighContention.hpp
  QueueTwoPartyNoCritical.hpp
  RecycleObjectStoreBitmap.hpp
  RecycleObjectStoreQueue.hpp
  RecycleObjectStoreVector.hpp
  Thread.hpp
  TimeUtils.hpp
  Tuple.hpp
) # Adding headers required for portability reasons http://voices.canonical.com/jussi.pakkanen/2013/03/26/a-list-of-common-cmake-antipatterns/
add_library(testing_lib ${TESTING_SRC})
set_target_properties(testing_lib PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(testing_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})