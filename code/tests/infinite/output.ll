; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

declare float @print_float(float)

define void @infinite() {
entry:
  br label %while_cond

while_cond:                                       ; preds = %while_body, %entry
  br i1 true, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  %call_tmp = call i32 @print_int(i32 1)
  br label %while_cond

while_end:                                        ; preds = %while_cond
  ret void
}
