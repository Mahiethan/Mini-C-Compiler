; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

define i32 @fibonacci(i32 %n) {
entry:
  %total = alloca i32, align 4
  %c = alloca i32, align 4
  %next = alloca i32, align 4
  %second = alloca i32, align 4
  %first = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %load_arg = load i32, ptr %n1, align 4
  %call_tmp = call i32 @print_int(i32 %load_arg)
  store i32 0, ptr %first, align 4
  store i32 1, ptr %second, align 4
  store i32 1, ptr %c, align 4
  store i32 0, ptr %total, align 4
  br label %while_cond

while_cond:                                       ; preds = %if_end, %entry
  %load_temp = load i32, ptr %c, align 4
  %load_temp2 = load i32, ptr %n1, align 4
  %lt_tmp = icmp slt i32 %load_temp, %load_temp2
  %if_cond = icmp ne i1 %lt_tmp, false
  br i1 %if_cond, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  %load_temp3 = load i32, ptr %c, align 4
  %le_tmp = icmp sle i32 %load_temp3, 1
  %if_cond4 = icmp ne i1 %le_tmp, false
  br i1 %if_cond4, label %if_then, label %if_else

if_then:                                          ; preds = %while_body
  %load_temp5 = load i32, ptr %c, align 4
  store i32 %load_temp5, ptr %next, align 4
  br label %if_end

if_else:                                          ; preds = %while_body
  %load_temp6 = load i32, ptr %first, align 4
  %load_temp7 = load i32, ptr %second, align 4
  %add_tmp = add i32 %load_temp6, %load_temp7
  store i32 %add_tmp, ptr %next, align 4
  %load_temp8 = load i32, ptr %second, align 4
  store i32 %load_temp8, ptr %first, align 4
  %load_temp9 = load i32, ptr %next, align 4
  store i32 %load_temp9, ptr %second, align 4
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %load_arg10 = load i32, ptr %next, align 4
  %call_tmp11 = call i32 @print_int(i32 %load_arg10)
  %load_temp12 = load i32, ptr %c, align 4
  %add_tmp13 = add i32 %load_temp12, 1
  store i32 %add_tmp13, ptr %c, align 4
  %load_temp14 = load i32, ptr %total, align 4
  %load_temp15 = load i32, ptr %next, align 4
  %add_tmp16 = add i32 %load_temp14, %load_temp15
  store i32 %add_tmp16, ptr %total, align 4
  br label %while_cond

while_end:                                        ; preds = %while_cond
  %load_arg17 = load i32, ptr %total, align 4
  %call_tmp18 = call i32 @print_int(i32 %load_arg17)
  %load_temp19 = load i32, ptr %total, align 4
  ret i32 %load_temp19
}
