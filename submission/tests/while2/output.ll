; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

declare float @print_float(float)

define i32 @foo(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  br label %while_cond

while_cond:                                       ; preds = %if_end, %entry
  %load_temp = load i32, ptr %x1, align 4
  %lt_tmp = icmp slt i32 %load_temp, 10
  %if_cond = icmp ne i1 %lt_tmp, false
  br i1 %if_cond, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  %load_temp2 = load i32, ptr %x1, align 4
  %lt_tmp3 = icmp slt i32 %load_temp2, 5
  %if_cond4 = icmp ne i1 %lt_tmp3, false
  br i1 %if_cond4, label %if_then, label %if_else

if_then:                                          ; preds = %while_body
  %load_temp5 = load i32, ptr %x1, align 4
  %add_tmp = add i32 %load_temp5, 2
  store i32 %add_tmp, ptr %x1, align 4
  br label %if_end

if_else:                                          ; preds = %while_body
  %load_temp6 = load i32, ptr %x1, align 4
  %add_tmp7 = add i32 %load_temp6, 1
  store i32 %add_tmp7, ptr %x1, align 4
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  br label %while_cond

while_end:                                        ; preds = %while_cond
  %load_temp8 = load i32, ptr %x1, align 4
  ret i32 %load_temp8
}
