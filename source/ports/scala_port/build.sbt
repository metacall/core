lazy val commonSettings = Seq(
  scalaVersion := "2.13.4",
  version := "0.1.0-SNAPSHOT",
  organization := "com.metacall",
  organizationName := "metacall",
  scalacOptions ++= Seq(
    // Only for debugging purposes
    // "-Djava.compiler=NONE",
    "-feature",
    "-deprecation",
    "-Wunused:imports,patvars,privates,locals,explicits,implicits,params",
    "-Xlint",
    "-explaintypes",
    "-Wdead-code",
    "-Wextra-implicit",
    "-Wnumeric-widen",
    "-Wconf:cat=lint-byname-implicit:silent"
  ),
  scalacOptions in (Compile, console) := Seq.empty,
  libraryDependencies ++= Seq(
    "net.java.dev.jna" % "jna" % "5.6.0",
    "org.typelevel" %% "cats-core" % "2.3.1",
    "com.chuusai" %% "shapeless" % "2.3.3",
    "org.scalatest" %% "scalatest" % "3.2.2" % Test
  )
)

lazy val root = (project in file("."))
  .settings(commonSettings: _*)
  .settings(
    name := "metacall",
    fork in (Test / run) := true,
    parallelExecution in Test := false
  )
