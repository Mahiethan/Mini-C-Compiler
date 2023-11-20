; ModuleID = 'mini-c'
source_filename = "mini-c"

define i32 @returns(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  br label %while_cond

while_cond:                                       ; preds = %entry
  %load_temp = load i32, ptr %x1, align 4
  %eq_tmp = icmp eq i32 %load_temp, 1
  %if_cond = icmp ne i1 %eq_tmp, false
  br i1 %if_cond, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  ret i32 0

while_end:                                        ; preds = %while_cond
  %load_temp2 = load i32, ptr %x1, align 4
  %gt_tmp = icmp sgt i32 %load_temp2, 1
  %if_cond3 = icmp ne i1 %gt_tmp, false
  br i1 %if_cond3, label %if_then, label %if_else

if_then:                                          ; preds = %while_end
  ret i32 1

if_else:                                          ; preds = %while_end
  ret i32 2
}
