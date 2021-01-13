(import testament :prefix "" :exit true)
(import ../build/settee :as settee)


(deftest set-with-no-items
  (is (= 0 (length (settee/new)))))


(deftest set-with-two-items
  (def s1 (settee/new :foo :bar))
  (is (= 2 (length s1))))


(deftest frozen-set-with-no-items
  (is (= 0 (length (settee/frozen)))))


(deftest check-frozen
  (is (not (settee/frozen? (settee/new))))
  (is (settee/frozen? (settee/frozen))))


(deftest add-one-item-to-set
  (def s1 (settee/new :foo :bar))
  (is (= 3 (length (settee/add s1 :baz)))))


(deftest add-two-items-to-set
  (def s1 (settee/new :foo :bar))
  (is (= 4 (length (settee/add s1 :baz :qux)))))


(deftest add-one-item-to-frozen-set
  (def s1 (settee/frozen :foo :bar))
  (def s2 (settee/add s1 :baz))
  (is (= 2 (length s1)))
  (is (= 3 (length s2))))


(deftest remove-one-item-from-set
  (def s1 (settee/new :foo :bar))
  (is (= 2 (length (settee/remove s1 :baz))))
  (is (= 1 (length (settee/remove s1 :bar)))))


(deftest next-with-empty-set
  (def s1 (settee/new))
  (is (= nil (next s1))))


(deftest next-with-non-empty-set
  (def s1 (settee/new :foo :bar))
  (def k1 (next s1))
  (def k2 (next s1 k1))
  (is (= nil (next s1 k2))))


(deftest hashing-with-equivalent-settors
  (def h1 (hash (settee/new :foo :bar)))
  (def h2 (hash (settee/new :bar :foo)))
  (is (= h1 h2)))


(deftest hashing-with-different-settors
  (def h1 (hash (settee/new [:foo :bar])))
  (def h2 (hash (settee/new [:foo :baz])))
  (is (not (= h1 h2))))


(run-tests!)
