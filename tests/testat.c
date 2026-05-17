struct S{
	int n;
	char text[16];
	};
	
struct S a;
struct S v[10];

void f(char text[],int i,char ch){
	text[i]=ch;
	//1=ch; -exprAssign
	//ch[i]=text; -exprPostfix
	//return 1; -stm
	//if(a||1) -exprOr
	//if(a&&1) -exprAnd
	//if(a==1) -exprEq
	//if(a>1) -exprRel
	//a+1=2; -exprAdd
	//a/1=2; -exprMul
	}

int h(int x,int y){
	if(x>0&&x<y){
		f(v[x].text,y,'#');
		//f(x.text,y,'#'); -exprPostfix
		//f(v[y].m,y,'#'); -exprPostfix
		//f(v[x].text,y,'#',1); -exprPrimary
		//f(v[x].text,y); - exprPrimary
		return 1;
		}
	return 0;
	}
