(define-module (repl)
  #:use-module (serveez-mg core)
  #:use-module (serveez-mg lib)
  #:use-module (rnrs bytevectors)
  #:use-module (ice-9 scm-style-repl)
  #:export (spawn-repl))

(define xxserver #f)
(define xxsocket #f)

(define (spawn-repl xserver xsocket)
  (set! xxserver xserver)
  (set! xxsocket xsocket)
  (export 'xxserver)
  (export 'xxsocket)
  (scm-style-repl))
