ThisBuild / scalaVersion := "2.13.4"
ThisBuild / version := "0.1.0-SNAPSHOT"
ThisBuild / organization := "com.metacall"
ThisBuild / organizationName := "metacall"

lazy val root = (project in file("."))
  .settings(
    name := "metacall",
    scalacOptions ++= Seq(
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
      "org.typelevel" %% "cats-effect" % "2.3.1",
      "org.scalatest" %% "scalatest" % "3.2.2" % Test
    ),
    fork in (Test / run) := true
  )
