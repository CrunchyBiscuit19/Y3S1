(define Sigma0
  (lambda (n f)
    (letrec ([visit (lambda (i)
                      (if (= i 0)
                          (f 0)
                          (+ (f i) (visit (- i 1)))))])
      (if (< n 0)
          0
          (visit n)))))

(define Sigma0_tail
  (lambda (n f)
    (letrec ([visit (lambda (i a)
                      (if (= i 0)
                          a
                          (visit (- i 1) (+ a (f i)))))])
      (if (< n 0)
          0
          (visit n (f 0))))))

(define one (lambda (a) 1))
(define identity (lambda (a) a))
(define square (lambda (a) (* a a)))

(define reference_one
  (lambda (n)
    (if (< n 0)
        0
        (+ n 1))))

(define reference_identity
  (lambda (n)
    (if (< n 0)
        0
        (/ (* n (+ n 1)) 2))))

(define reference_square
  (lambda (n)
    (if (< n 0)
        0
        (/ (* n (* (+ n 1) (+ (* 2 n) 1))) 6))))

(define test_Sigma
  (lambda (func)
    (letrec ([check (lambda (n f reference)
                      (= (func n f) (reference n)))]
             [loop (lambda (n)
                     (if (> n 10)
                         #t
                         (and (check n one reference_one)
                              (check n identity reference_identity)
                              (check n square reference_square)
                              (loop (+ n 1)))))])
      (loop -5))))

(define test_Sigmas_same
  (lambda (func1 func2)
    (letrec ([loop (lambda (n)
                     (if (> n 10)
                         #t
                         (and (= (func1 n square) (func2 n square))
                              (loop (+ n 1)))))])
      (loop -5))))

(test_Sigma Sigma0)
(test_Sigma Sigma0_tail)
(test_Sigmas_same Sigma0 Sigma0_tail)
