package metacall

import org.scalatest.Suites
import org.scalatest.BeforeAndAfterAll

class Main extends Suites(new MetaCallSpec) with BeforeAndAfterAll {

  override protected def beforeAll(): Unit = {
    require(
      Bindings.instance.metacall_initialize() == 0,
      "MetaCall could not initialize"
    )
  }

  override protected def afterAll(): Unit = {
    require(
      Bindings.instance.metacall_destroy() == 0,
      "MetaCall was not successfully destroyed"
    )
  }

}
