(declare-const ent0 Bool)
(declare-const ent1 Bool)
(declare-const ent2 Bool)
(declare-const ent3 Bool)
(declare-const ent4 Bool)
(declare-const ent5 Bool)
(declare-const ent6 Bool)
(declare-const ent7 Bool)
(declare-const ent8 Bool)
(declare-const ent9 Bool)
(declare-const ent10 Bool)
(declare-const ent11 Bool)
(declare-const ent12 Bool)
(declare-const ent13 Bool)
(declare-const ent14 Bool)
(declare-const ent15 Bool)
(declare-const reac0 Bool)
(declare-const dir0 Bool)
(declare-const coef0 Int)
(declare-const reac1 Bool)
(declare-const dir1 Bool)
(declare-const coef1 Int)
(declare-const reac2 Bool)
(declare-const dir2 Bool)
(declare-const coef2 Int)
(declare-const reac3 Bool)
(declare-const dir3 Bool)
(declare-const coef3 Int)
(declare-const reac4 Bool)
(declare-const dir4 Bool)
(declare-const coef4 Int)
(declare-const reac5 Bool)
(declare-const dir5 Bool)
(declare-const coef5 Int)
(declare-const reac6 Bool)
(declare-const dir6 Bool)
(declare-const coef6 Int)
(declare-const reac7 Bool)
(declare-const dir7 Bool)
(declare-const coef7 Int)
(declare-const reac8 Bool)
(declare-const dir8 Bool)
(declare-const coef8 Int)
(declare-const reac9 Bool)
(declare-const dir9 Bool)
(declare-const coef9 Int)
(declare-const reac10 Bool)
(declare-const dir10 Bool)
(declare-const coef10 Int)
(declare-const reac11 Bool)
(declare-const dir11 Bool)
(declare-const coef11 Int)
(assert (= dir0 (< coef0 0)))
(assert (= dir1 (< coef1 0)))
(assert (= dir2 (< coef2 0)))
(assert (= dir3 (< coef3 0)))
(assert (= dir4 (< coef4 0)))
(assert (= dir5 (< coef5 0)))
(assert (= dir6 (< coef6 0)))
(assert (= dir7 (< coef7 0)))
(assert (= dir8 (< coef8 0)))
(assert (= dir9 (< coef9 0)))
(assert (= dir10 (< coef10 0)))
(assert (= dir11 (< coef11 0)))
(assert (=> reac0 (and(or ent3 ent0) ent6)))
(assert (=> reac1 (and(or ent7 ent4) ent6)))
(assert (=> reac2 (and(or ent2 ent2) ent4)))
(assert (=> reac3 (and(or ent11 ent1) ent14)))
(assert (=> reac4 (and(or ent15 ent12) ent14)))
(assert (=> reac5 (and(or ent10 ent10) ent12)))
(assert (=> reac6 (and(or ent2 ent15) ent8)))
(assert (=> reac7 (and(or ent10 ent7) ent9)))
(assert (=> reac8 (and(or ent2 ent0) ent5)))
(assert (=> reac9 (and(or ent7 ent3) ent5)))
(assert (=> reac10 (and(or ent10 ent1) ent13)))
(assert (=> reac11 (and(or ent15 ent11) ent13)))
(assert (=> (and reac0 (not dir0)) (or (not ent3) (not ent0))))
(assert (=> (and reac1 (not dir1)) (or (not ent7) (not ent4))))
(assert (=> (and reac3 (not dir3)) (or (not ent11) (not ent1))))
(assert (=> (and reac4 (not dir4)) (or (not ent15) (not ent12))))
(assert (=> (and reac6 (not dir6)) (or (not ent2) (not ent15))))
(assert (=> (and reac7 (not dir7)) (or (not ent10) (not ent7))))
(assert (=> (and reac8 (not dir8)) (or (not ent2) (not ent0))))
(assert (=> (and reac9 (not dir9)) (or (not ent7) (not ent3))))
(assert (=> (and reac10 (not dir10)) (or (not ent10) (not ent1))))
(assert (=> (and reac11 (not dir11)) (or (not ent15) (not ent11))))
(assert (=> ent0 (> (+ (ite reac0 (- coef0) 0) (ite reac8 (- coef8) 0) 0) 0)))
(assert (=> ent1 (> (+ (ite reac3 (- coef3) 0) (ite reac10 (- coef10) 0) 0) 0)))
(assert (=> ent2 (> (+ (ite reac2 (+ (- coef2) (- coef2)) 0) (ite reac6 (- coef6) 0) (ite reac8 (- coef8) 0) 0) 0)))
(assert (=> ent3 (> (+ (ite reac0 (- coef0) 0) (ite reac9 (- coef9) 0) 0) 0)))
(assert (=> ent4 (> (+ (ite reac1 (- coef1) 0) (ite reac2 coef2 0) 0) 0)))
(assert (=> ent5 (> (+ (ite reac8 coef8 0) (ite reac9 coef9 0) 0) 0)))
(assert (=> ent6 (> (+ (ite reac0 coef0 0) (ite reac1 coef1 0) 0) 0)))
(assert (=> ent7 (> (+ (ite reac1 (- coef1) 0) (ite reac7 (- coef7) 0) (ite reac9 (- coef9) 0) 0) 0)))
(assert (=> ent8 (> (+ (ite reac6 coef6 0) 0) 0)))
(assert (=> ent9 (> (+ (ite reac7 coef7 0) 0) 0)))
(assert (=> ent10 (> (+ (ite reac5 (+ (- coef5) (- coef5)) 0) (ite reac7 (- coef7) 0) (ite reac10 (- coef10) 0) 0) 0)))
(assert (=> ent11 (> (+ (ite reac3 (- coef3) 0) (ite reac11 (- coef11) 0) 0) 0)))
(assert (=> ent12 (> (+ (ite reac4 (- coef4) 0) (ite reac5 coef5 0) 0) 0)))
(assert (=> ent13 (> (+ (ite reac10 coef10 0) (ite reac11 coef11 0) 0) 0)))
(assert (=> ent14 (> (+ (ite reac3 coef3 0) (ite reac4 coef4 0) 0) 0)))
(assert (=> ent15 (> (+ (ite reac4 (- coef4) 0) (ite reac6 (- coef6) 0) (ite reac11 (- coef11) 0) 0) 0)))
(assert ((_ at-most 12) ent0 ent1 ent2 ent3 ent4 ent5 ent6 ent7 ent8 ent9 ent10 ent11 ent12 ent13 ent14 ent15))
(assert ((_ at-least 12) ent0 ent1 ent2 ent3 ent4 ent5 ent6 ent7 ent8 ent9 ent10 ent11 ent12 ent13 ent14 ent15))
(assert ((_ at-least 12) reac0 reac1 reac2 reac3 reac4 reac5 reac6 reac7 reac8 reac9 reac10 reac11))
(assert ((_ at-most 12) reac0 reac1 reac2 reac3 reac4 reac5 reac6 reac7 reac8 reac9 reac10 reac11))
(assert (not (and reac3 (not dir3) reac4 dir4 reac5 dir5 reac10 (not dir10) reac11 dir11 ent10 ent11 ent12 ent13 ent14)))
(assert (not (and reac0 (not dir0) reac1 dir1 reac2 dir2 reac8 (not dir8) reac9 dir9 ent2 ent3 ent4 ent5 ent6)))
(check-sat)
(get-model)
