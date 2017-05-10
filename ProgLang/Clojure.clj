;Luke Kledzik
;Programming Languages
;Project 3

;params: the row num of pascal's triangle, then the coefficient num on that row
;returns: coefficient value at that location
(defn bico [i j]
    (if (== j 0) 1
        (if (== j i) 1
            (+ (bico (- i 1) (- j 1)) (bico (- i 1) j))
        )
    )
)

;helper function for egypt
(defn times2 [val]
    (+ val val)
)

;helper function for egypt
(defn halve
    ([val] (halve (- val 1) 0))
    ([val ans] (if (> val 0) (halve (- val 2) (+ ans 1)) ans))
)

;completes division via egyptian method
;no * or / operators used
(defn egypt
    ([a b]
        (egypt 1 b a)
    )
    ([a b c]
        (if (< b c) (egypt (times2 a) (times2 b) c) (egypt a b c 0))
    )
    ([a b c d]
        (if (< a 1) d
            (if (<= b c) (egypt a b (- c b) (+ a d)) (egypt (halve a) (halve b) c d))
        )
    )
)

;helper function for primes
(defn exp [x n]
    (reduce * (repeat n x))
)

;should work correctly, not sure how to print the output though
(defn primes
	([] (primes 1 1))
	([n m] (if (= n 1) (lazy-seq (cons (* (exp 7 n) (exp 11 m)) (primes (+ m 1) 1)))
        (lazy-seq (cons (* (exp 7 n) (exp 11 m)) (primes (- n 1) (+ m 1)))))
    )
)

(println (bico 4 2))
(println (egypt 1960 56))
(take 4 (primes))
