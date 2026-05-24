struct S{
	int n;
	char text[16];
	};
	
struct S a;
struct S v[10];

void f(char text[],int i,char ch){
	text[i]=ch;
	//if(text){} -stm IF
	//while(text){} -stm WHILE
	//return 1; -stm Return void
	//1=ch; -exprAssign left-value
	//i=v; -exprAssign scalar
	//i=a; //-exprAssign sursa - destinatie
	//ch[i]=text; -exprPostfix
	//if(a||1) -exprOr
	i=(i||1);
	//if(a&&1) -exprAnd
	i=(i&&1);
	//if(a==1) -exprEq
	i=(i==1);
	//if(a>1) -exprRel
	i=(i<=1);
	//a+1=2; -exprAdd
	i=i+1;
	//a/1=2; -exprMul
	i=i/2;
	//i=(int)a; -exprCast struct nu se poate converti
	//i=(struct S)a; -exprCast nu poti converti la struct
	//text[0]=(char)text; - exprCast array to array
	//i=(char[])text; -exprCast scalar to scalar
	//i=-text; //-exprUnary
	//i[5]=text; //-exprPostfix - array indexing
	//i=a.text[a]; -exprPostfix - cannot convert to int
	}

int h(int x,int y){
	if(x>0&&x<y){
		f(v[x].text,y,'#');
		//f(x.text,y,'#'); -exprPostfix - field not from struct
		//f(v[y].m,y,'#'); -exprPostfix - unexistent field
		//p(v[x].text,y,'#'); -exprPrimary - ID inexistend
		//a(v[x].text,y,'#') -exprPrimary - only functions can be called
		//f(v[x].text,y,'#',1); //-exprPrimary parameteres
		//f(v[x].text,y); - exprPrimary parameters		
		//f(a,y,'#'); - exprPrimary can't convert struct
		//return v; -stm return scalar
		//return; -stm return non-void
		return 1;
		}
	return 0;
	}
