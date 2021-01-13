(declare-project
  :name "Settee"
  :description "A set data structure for Janet"
  :author "Michael Camilleri"
  :license "MIT"
  :url "https://github.com/pyrmont/settee"
  :repo "git+https://github.com/pyrmont/settee"
  :dependencies ["https://github.com/pyrmont/testament"])


(def cflags
  (case (os/which)
    :windows []
    ["-std=c99" "-Wall" "-Wextra" "-O3"]))


(def lflags
  [])


(declare-native
  :name "settee"
  :cflags cflags
  :lflags lflags
  :headers ["src/settee.h"]
  :source ["src/settee.c"])
