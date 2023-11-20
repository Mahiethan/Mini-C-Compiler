; ModuleID = 'mini-c'
source_filename = "mini-c"

@mutable_var = common global i32 0, align 4

define i32 @mutating_function() {
entry:
  %load_global_temp = load i32, ptr @mutable_var, align 4
  %add_tmp = add i32 %load_global_temp, 1
  store i32 %add_tmp, ptr @mutable_var, align 4
  ret i32 1
}

define i32 @lazyeval_and(i32 %control) {
entry:
  %control1 = alloca i32, align 4
  store i32 %control, ptr %control1, align 4
  store i32 0, ptr @mutable_var, align 4
  br i1 false, label %if_then, label %if_else

if_then:                                          ; preds = %entry
  %load_global_temp = load i32, ptr @mutable_var, align 4
  ret i32 %load_global_temp

if_else:                                          ; preds = %entry
  %load_global_temp2 = load i32, ptr @mutable_var, align 4
  ret i32 %load_global_temp2
}

define i32 @lazyeval_or(i32 %control) {
entry:
  %control1 = alloca i32, align 4
  store i32 %control, ptr %control1, align 4
  store i32 0, ptr @mutable_var, align 4
  br i1 true, label %if_then, label %if_else

if_then:                                          ; preds = %entry
  %load_global_temp = load i32, ptr @mutable_var, align 4
  ret i32 %load_global_temp

if_else:                                          ; preds = %entry
  %load_global_temp2 = load i32, ptr @mutable_var, align 4
  ret i32 %load_global_temp2
}
