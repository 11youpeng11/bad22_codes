/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <limits.h>
#include "memory/paddr.h"
#include "memory/vaddr.h"
#include <string.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, 
 
  /* TODO: Add more token types */
  TK_NUM = 1,	//0-9
  TK_REGISTER = 2,	//寄存器
  TK_HEX = 3,	//十六进制
  TK_EQ = 4,	
  TK_NOTEQ = 5,
  TK_OR = 6,
  TK_AND = 7,
  TK_LEFT = 8,
  TK_RIGHT = 9,
  TK_LEQ = 10,
  TK_POINT,	//指针
  TK_NEG	//负数

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  
  {"\\-", '-'},         // sub
  {"\\*", '*'},         // mul
  {"\\/", '/'},         // div
  
  {"\\(", TK_LEFT},         
  {"\\)", TK_RIGHT},   
  
  {"\\<\\=", TK_LEQ},
  {"\\!\\=", TK_NOTEQ},
   {"==", TK_EQ},        // equal
   
  {"\\&\\&", TK_AND},
  {"\\|\\|", TK_OR},
  {"\\!", '!'},
  
  {"\\$[a-zA-Z]*[0-9]*", TK_REGISTER},
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]*",TK_NUM},
};

#define NR_REGEX ARRLEN(rules)

//bool division_zero = false;

static regex_t re[NR_REGEX] = {};

/* 
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

int len = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
	
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
	
	Token tmp_token;
	
        switch (rules[i].token_type) {
        	case '+':
			tmp_token.type = '+';
			tokens[nr_token] = tmp_token;
			nr_token ++;
			break;
		    case '-':
		    	
			tmp_token.type = '-';
			tokens[nr_token] = tmp_token;
			nr_token ++;
			break;
		    case '*':
			tmp_token.type = '*';
			tokens[nr_token] = tmp_token;
			nr_token ++;
			break;
		    case '/':
			tmp_token.type = '/';
			tokens[nr_token] = tmp_token;
			nr_token ++;
			break;
		    case 256:
			break;
		    case '!':
			tmp_token.type = '!';
			tokens[nr_token] = tmp_token;
			nr_token ++;
			break;
		    case 9:
			tmp_token.type = ')';
			tokens[nr_token] = tmp_token;
			nr_token ++;
			break;
		    case 8:
			tmp_token.type = '(';
			tokens[nr_token] = tmp_token;
			nr_token ++;
			break;
		    case 10:
			tokens[nr_token].type = 10;
			strcpy(tokens[nr_token].str, "<=");
			nr_token ++;
			break;
		    case 4:
			tokens[nr_token].type = 4;
			strcpy(tokens[nr_token].str, "==");
			nr_token++;
			break;
		    case 5:
			tokens[nr_token].type = 5;
			strcpy(tokens[nr_token].str, "!=");
			nr_token++;
			break;
		    case 6:
			tokens[nr_token].type = 6;
			strcpy(tokens[nr_token].str, "||");
			nr_token++;
			break;
		    case 7:
			tokens[nr_token].type = 7;
			strcpy(tokens[nr_token].str, "&&");
			nr_token++;
			break;
		    
			
		    case 1: 
			tokens[nr_token].type = 1;
			strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
			nr_token ++;
			break;
		    case 2: 
			tokens[nr_token].type = 2;
			strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
			nr_token ++;
			break;
		    case 3: 
			tokens[nr_token].type = 3;
			strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
			nr_token ++;
			break;
		    
		    
		    default:
			printf("i = %d and No rules is com.\n", i);
			break;
		}
		len = nr_token;		//更新
		break;
	    }
	}

    	if (i == NR_REGEX) {
     		printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      		return false;
    	}
    }
   return true; 
}

bool check_parentheses(int p, int q){
    if(tokens[p].type != '('  || tokens[q].type != ')') {
        return false;
    }
    int l = p , r = q;
    while(l < r) {
	if(tokens[l].type == '(') {
		if(tokens[r].type == ')') {
			l++ , r--;
                	continue;
            	} else {
                	r--;
            	}
		} else if(tokens[l].type == ')') {
            		return false;
        	} else {
            		l++;
        	}
    }
    return true;
}

int find_main_operator(int p, int q) {
    int parentheses_count = 0;    // 记录括号的个数
    int lowest_priority = INT_MAX;    // 当前找到的主运算符的最低优先级
    int main_operator = -1;    // 主运算符的位置

    for (int i = p; i <= q; i++) {
        if (tokens[i].type == '(') {
            parentheses_count++;    // 遇到左括号，括号的个数加一
        } else if (tokens[i].type == ')') {
            parentheses_count--;    // 遇到右括号，括号的个数减一
        }

        if (parentheses_count == 0) {
            if (tokens[i].type == '+' || tokens[i].type == '-') {
                if (lowest_priority >= 1) {
                    lowest_priority = 1;    // 更新最低优先级
                    main_operator = i;    // 更新主运算符的位置
                }
            } else if (tokens[i].type == '*' || tokens[i].type == '/') {
                if (lowest_priority >= 2) {
                    lowest_priority = 2;    // 更新最低优先级
                    main_operator = i;    // 更新主运算符的位置
                }
            }
        }
    }
    return main_operator;    // 返回主运算符的位置
}

uint32_t eval(int p, int q) {
	if (p > q) {
        	// 错误的表达式，抛出断言错误
		assert(0);
	} else if (p == q) {
        	
        	return  atoi(tokens[p].str);
    	} else if (check_parentheses(p, q)) {
        	// 表达式被一对括号包围，去掉括号并递归求值
        	return eval(p + 1, q - 1);
    	} else {
        	int op = find_main_operator(p, q);    // 找到主运算符的位置
        	uint32_t val1 = eval(p, op - 1);    // 对主运算符左边的表达式进行求值
        	uint32_t val2 = eval(op + 1, q);    // 对主运算符右边的表达式进行求值
		
		if (tokens[op].type == TK_NEG) {
			return -eval(op + 1, q);  // 对负数进行取反操作
		}
		if (tokens[op].type == TK_POINT) {
			uint32_t addr = eval(op + 1, q);  // 求解指针的地址
			return vaddr_read(addr, 4);      // 读取指针地址处的值
		}
        	switch (tokens[op].type) {
            		case '+':
                		return val1 + val2;    // 执行加法运算并返回结果
            		case '-':
                		return val1 - val2;    // 执行减法运算并返回结果
            		case '*':
                		return val1 * val2;    // 执行乘法运算并返回结果
            		case '/':
                		if (val2 == 0) {
            
                    		return 0;
                		} else {
                    			return val1 / val2;    // 执行除法运算并返回结果
                		}
	    		case 4:
				return val1 == val2;
	    		case 5:
				return val1 != val2;
	    		case 6:
				return val1 || val2;
	    		case 7:
				return val1 && val2;
            		default:
            			printf("No op type.\n");
               			assert(0);    // 未知的运算符类型，抛出断言错误
		}
		
	
	}
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
	
	*success = true;
	int tokens_len = len;
	
	for (int i = 0; i < tokens_len; i++) {
    		// 判断当前标记是否为负号，并且前一个标记不是数字、十六进制数、右括号或寄存器
		if ((tokens[i].type == '-') && 
			(i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_RIGHT && tokens[i - 1].type != TK_REGISTER))) {
      			tokens[i].type = TK_NEG;  // 将负号标记为TK_NEGATIVE
   		}
    		// 判断当前标记是否为星号，并且前一个标记不是数字、十六进制数、右括号或寄存器
    		if ((tokens[i].type == '*' )&& 
    			(i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_RIGHT && tokens[i - 1].type != TK_REGISTER))) {
      			tokens[i].type = TK_POINT;  // 将星号标记为TK_POINT
    		}
	}
	
	printf("标记个数为: %d\n", tokens_len);
	
	
	return eval(0, tokens_len - 1);
	
}




