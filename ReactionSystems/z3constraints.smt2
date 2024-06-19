(declare-const ent0 Bool)
(declare-const ent1 Bool)
(declare-const ent2 Bool)
(declare-const ent3 Bool)
(declare-const ent4 Bool)
(declare-const ent5 Bool)
(declare-const ent6 Bool)
(declare-const ent7 Bool)
(declare-const ent8 Bool)
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
(assert (= dir0 (< coef0 0)))
(assert (= dir1 (< coef1 0)))
(assert (= dir2 (< coef2 0)))
(assert (= dir3 (< coef3 0)))
(assert (= dir4 (< coef4 0)))
(assert (= dir5 (< coef5 0)))
(assert (= dir6 (< coef6 0)))
(assert (=> reac0 (and(or ent0 ent2)(or ent3))))
(assert (=> reac1 (and(or ent3 ent1)(or ent5))))
(assert (=> reac2 (and(or ent5 ent0)(or ent6))))
(assert (=> reac3 (and(or ent6 ent1)(or ent7))))
(assert (=> reac4 (and(or ent8 ent0)(or ent7))))
(assert (=> reac5 (and(or ent2 ent4)(or ent8))))
(assert (=> reac6 (and(or ent2 ent1)(or ent4))))
(assert (=> ent0 (> (+ (ite reac0 (* -1 coef0) 0) (ite reac2 (* -1 coef2) 0) (ite reac4 (* -1 coef4) 0) 0) 0)))
(assert (=> ent1 (> (+ (ite reac1 (* -1 coef1) 0) (ite reac3 (* -1 coef3) 0) (ite reac6 (* -1 coef6) 0) 0) 0)))
(assert (=> ent2 (> (+ (ite reac0 (* -1 coef0) 0) (ite reac5 (* -1 coef5) 0) (ite reac6 (* -1 coef6) 0) 0) 0)))
(assert (=> ent3 (> (+ (ite reac0 (* 1 coef0) 0) (ite reac1 (* -1 coef1) 0) 0) 0)))
(assert (=> ent4 (> (+ (ite reac5 (* -1 coef5) 0) (ite reac6 (* 1 coef6) 0) 0) 0)))
(assert (=> ent5 (> (+ (ite reac1 (* 1 coef1) 0) (ite reac2 (* -1 coef2) 0) 0) 0)))
(assert (=> ent6 (> (+ (ite reac2 (* 1 coef2) 0) (ite reac3 (* -1 coef3) 0) 0) 0)))
(assert (=> ent7 (> (+ (ite reac3 (* 1 coef3) 0) (ite reac4 (* 1 coef4) 0) 0) 0)))
(assert (=> ent8 (> (+ (ite reac4 (* -1 coef4) 0) (ite reac5 (* 1 coef5) 0) 0) 0)))
(assert ((_ at-most 7) ent0 ent1 ent2 ent3 ent4 ent5 ent6 ent7 ent8))
(assert ((_ at-least 7) ent0 ent1 ent2 ent3 ent4 ent5 ent6 ent7 ent8))
(assert ((_ at-least 7) reac0 reac1 reac2 reac3 reac4 reac5 reac6))
(assert ((_ at-most 7) reac0 reac1 reac2 reac3 reac4 reac5 reac6))
(assert (not (and reac0 dir0 reac1 dir1 reac2 dir2 reac3 dir3 reac4 (not dir4) ent0 ent3 ent5 ent6 ent7)))
(assert (not (and reac1 dir1 reac2 dir2 reac3 dir3 reac4 (not dir4) reac5 (not dir5) reac6 (not dir6) ent1 ent4 ent5 ent6 ent7 ent8)))
(assert (not (and reac0 (not dir0) reac1 (not dir1) reac2 (not dir2) reac3 (not dir3) reac4 dir4 reac5 dir5 reac6 dir6 ent2 ent3 ent4 ent5 ent6 ent7 ent8)))
(check-sat)
(get-model)
