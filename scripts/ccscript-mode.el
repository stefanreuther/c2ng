;;; ccscript-mode --- Mode for editing CCScript files

;; (c) copyright 2001,2010,2016 Stefan Reuther

;; Author: Stefan Reuther <Streu@gmx.de>
;; X-URL: http://phost.de/~stefan/pcc.html

;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License as
;; published by the Free Software Foundation; either version 2, or (at
;; your option) any later version.

;; This program is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with [your] Emacs; see the file COPYING.  If not, write to the
;; Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA.

;;; Commentary:

;;  CCScript is the scripting language of Planets Command Center,
;;  a player-side program for the play-by-email game `VGA Planets'.
;;  This file provides a major mode for editing such files.

;;  This version is a rewrite of the original mode.
;;  It uses more Emacs infrastructure for highlighting, and supports indentation.
;;  It no longer tries to automatically convert case, or file formats.

;;; Usage:

;;     (load "ccscript.el")
;;  in ~/.emacs, and you're ready. This associates the extension `.q'
;;  with CCScript mode. You can also M-x ccscript-mode manually.

;;; History:

;;  20/Jan/2001  started
;;  20/May/2001  Name lists updated for PCC 1.0.11
;;               Bugfix: don't mess up case in literals/comments
;;               Bugfix: parsing parenthesized expressions didn't work
;;  08/Dec/2002  Name lists updated from PCC 1.1.3
;;  27/Oct/2004  Name lists updated from PCC 1.1.9
;;               Added some customize declarations
;;  26/Apr/2010  Added some PCC2 keywords
;;  19/Jun/2010  ccscript-electric-punct honors prefix arg
;;  07/Aug/2010  Brute-force assembler support (ccasm-mode)
;;  12/Nov/2016  Rewrite

(defvar ccscript-mode-hook nil)

(defvar ccscript-mode-map
  (let ((map (make-keymap)))
    ;(define-key map "\C-j" 'newline-and-indent)
    map)
  "Keymap for CCScript major mode")

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.q\\'" . ccscript-mode))

(defconst ccscript-font-lock-keywords-1
  (list
   (cons (concat "\\<" (regexp-opt '("Abort" "And" "Break" "Case" "Close" "Continue" "Dim" "Do" "Else" "End"
                                     "EndFunction" "EndIf" "EndSelect" "EndSub" "EndTry" "EndWith" "For" 
                                     "ForEach" "Function" "If" "Is" "Local" "Loop" "Mod" "Next" "Not" "On" "Open"
                                     "Option" "Optional" "Or" "Return" "RunHook" "Select" "Shared" "Static"
                                     "Stop" "Sub" "To" "Then" "Try" "Until" "While" "With" "Xor"))
                 "\\>")
         'font-lock-keyword-face)))


(defvar ccscript-font-lock-keywords ccscript-font-lock-keywords-1)

;;
;;  Parsing
;;
(defun ccscript-check-re (RE)
  "Skip over regexp

Like `looking-at', but places point after match if successful"
  (if (looking-at RE)
      (goto-char (match-end 0))
    nil)
  )

(defun ccscript-check-paren ()
  "Skip over CCScript parenthesized operand list

If looking at a CCScript operand, skip it forward and return t.
Otherwise, return nil."
  (while (or (ccscript-check-expr)
	     (ccscript-check-re "[ \t]*,[ \t]*")))
  (ccscript-check-re ")")
  )

(defun ccscript-check-operand ()
  "Skip over CCScript operand

If looking at a CCScript operand, skip it forward and return t.
Otherwise, return nil. Very simplified parsing"
  ;; skip unary operators
  (ccscript-check-re "\\([- \t+]\\|\\<not\\>\\)*")
  (let (ok)
    ;; identifier or number
    (if (ccscript-check-re "[a-z0-9._$]+")
	(setq ok t))
    ;; parenthesized expression
    (if (ccscript-check-re "(")
	(setq ok (or (ccscript-check-paren) ok)))
    ;; single-quoted string
    (if (ccscript-check-re "'[^'\n]*'")
	(setq ok t))
    ;; double-quoted string
    (if (ccscript-check-re "\"\\([^\"\\\n]*\\|\\[^\n]\\)\"")
	(setq ok t))
    ok)
  )

(defun ccscript-check-expr ()
  "Skip over CCScript expression

If looking at a CCScript expression, skip it forward and return t.
Otherwise, return nil."
  (if (ccscript-check-operand)
      (progn
	(while (and (ccscript-check-re "[ \t]*")
		    (ccscript-check-re "[-=.+*/\\&#^;]\\|\\<\\(mod\\|and\\|x?or\\)\\>\\|[<>:][=>]?")
		    (ccscript-check-operand)))
	t)
    nil)
  )

(defun ccscript-classify-current-line (NEW)
  "Classify current line for indentation of CCScript files

Return value is a cons (THIS . NEXT), where THIS is the indentation of
this line relative to the previous one, and NEXT is the indentation of
the next line relative to this one. Values are multiplied by
`ccscript-basic-indent', so sensible values for THIS and NEXT are
-1, 0 and 1.

Return value may also be NIL to cause this line to be ignored during
indentation.

NEW says whether this line is the one we're indenting (t)
or not (nil)"
  (let ((case-fold-search t))
    (save-excursion
      (beginning-of-line)
      (ccscript-check-re "[ \t]+")	; skip leading whitespace
      (cond
       ;; Indent line after `Do' or `Sub'
       ((looking-at "\\<\\(do\\|sub\\|function\\)\\>")
	'(0 . 1))
       ;; Indent line after `Local Sub'
       ((looking-at "\\<local[ \t]+\\(sub\\|function\\)\\>")
	'(0 . 1))
       ;; Outdent `EndWith', `EndSub' etc.
       ((looking-at "\\<\\(endwith\\|endsub\\|endfunction\\|endtry\\|endif\\|next\\|loop\\)\\>")
	'(-1 . 0))
       ;; Indent after `ForEach' or `With' if this is a multiline loop
       ((and (ccscript-check-re "\\<\\(foreach\\|with\\)\\>")
	     (ccscript-check-expr))
	(ccscript-check-re "\\<do\\>")
	(if (or (ccscript-check-re "[ \t]*$")
		(ccscript-check-re "[ \t]*%"))
	    '(0 . 1)
	  '(0 . 0)))
       ;; Indent *twice* after `Select'
       ((looking-at "\\<select\\>")
        '(0 . 2))
       ((looking-at "\\<case\\>")
        '(-1 . 1))
       ((looking-at "\\<endselect\\>")
        '(-2 . 0))
       ;; Indent after `If' if this is a multi-line statement
       ((and (ccscript-check-re "\\<\\(if\\)\\>")
	     (ccscript-check-expr))
	(ccscript-check-re "\\<then\\>")
	(if (or (ccscript-check-re "[ \t]*$")
		(ccscript-check-re "[ \t]*%"))
	    '(0 . 1)
	  '(0 . 0)))
       ;; Indent after multiline `For'
       ((and (ccscript-check-re "\\<\\(for\\)\\>")
	     (ccscript-check-expr)
	     (ccscript-check-re "\\<\\(to\\)\\>")
	     (ccscript-check-expr))
	(ccscript-check-re "\\<\\(do\\)\\>")
	(if (or (ccscript-check-re "[ \t]*$")
		(ccscript-check-re "[ \t]*%"))
	    '(0 . 1)
	  '(0 . 0)))
       ;; Outdent `Else' once, but indent again after
       ((looking-at "\\<else\\>")
	'(-1 . 1))
       ;; Indent multi-line `Try'
       ((ccscript-check-re "\\<try\\>[ \t]*")
	(if (looking-at "$\\|%")
	    '(0 . 1)
	  '(0 . 0)))
       ;; Ignore blank lines
       ((looking-at "$")
	(if NEW '(0 . 0) nil))
       ;; Ignore comments if at beginning of line; indent them
       ;; like normal statements otherwise
       ((looking-at "%")
	(if (bolp)
	    nil
	  '(0 . 0)))
       ;; Assembler: outdent labels
       ((looking-at "[^ \t]+:\\([^=]\\|$\\)")
        '(-1 . 1))
       ;; normal command
       (t
	'(0 . 0))
       )
      )
    )
  )


(defun ccscript-indent-line ()
  "Indent current line as CCScript"
  (interactive)
  (interactive)
  (let (thisind thiscode prevind prevcode new)
    (save-excursion
      (beginning-of-line)
      ;; get indentation of this line. If this line doesn't say anything
      ;; interesting, move up
      (setq new t)
      (while (and (not (setq thiscode (ccscript-classify-current-line new)))
		  (equal (forward-line -1) 0))
	(setq new nil))
      (setq thisind (current-indentation))
      ;; get indentation of the line before.
      (while (and (equal (forward-line -1) 0)
		  (not (setq prevcode (ccscript-classify-current-line nil)))))
      (if prevcode
	  (setq prevind (current-indentation))
	(setq prevind 0
	      prevcode '(0 . 0)))
      )
    ;; this line must be indented to prevind + cdr prevcode + car thiscode
    (indent-line-to (max 0
                         (+ prevind
                            (* ccscript-basic-indent (cdr prevcode))
                            (* ccscript-basic-indent (or (car thiscode) 0))))
                    )
    )
  )

(defcustom ccscript-basic-indent 2
  "Basic indentation for CCScript mode"
  :group 'ccscript
  :type 'integer)

(defvar ccscript-syntax-table
  (let ((st (make-syntax-table)))
    ;; "$" is a word character
    (modify-syntax-entry ?$ "w" st)
    ;; "%" is comment starter
    (modify-syntax-entry ?% "<" st)
    (modify-syntax-entry ?\n ">" st)
    ;; Both '' and "" quote
    (modify-syntax-entry ?' "\"" st)
    (modify-syntax-entry ?\" "\"" st)
    st)
  "Syntax table for ccscript-mode"
  )

(defun ccscript-mode ()
  "Major mode for editing CCScript files"
  (interactive)
  (kill-all-local-variables)
  (set-syntax-table ccscript-syntax-table)
  (use-local-map ccscript-mode-map)
  (set (make-local-variable 'font-lock-defaults) '(ccscript-font-lock-keywords))
  (set (make-local-variable 'indent-line-function) 'ccscript-indent-line)
  (setq major-mode 'ccscript-mode
        mode-name "CCScript"
        case-fold-search t
        comment-start "% "
        comment-end ""
        comment-start-skip "%+ *"
        )
  (run-hooks 'ccscript-mode-hook))

;(makunbound 'ccscript-font-lock-keywords)
;(makunbound 'ccscript-syntax-table)
