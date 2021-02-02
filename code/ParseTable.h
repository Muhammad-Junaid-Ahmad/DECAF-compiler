#pragma once
char ParseData[][200] = { "Ident	IntConst	DoubleConst	BoolConst	StringConst	null	int	double	bool	string	if	else	while	return	*	+	<	=	(	)	{	}	,	;	$",
	"Program							Decl Prog	Decl Prog	Decl Prog	Decl Prog															Sync",
	"Prog							Program	Program	Program	Program															?",
	"Decl							Type Ident RFDecl	Type Ident RFDecl	Type Ident RFDecl	Type Ident RFDecl															Sync",
	"RFDecl							Sync	Sync	Sync	Sync									( Formals ) ^A14 StmtBlock ^A15					;	Sync",
	"Variable							Type Ident	Type Ident	Type Ident	Type Ident										Sync			Sync	Sync	",
	"Type	Sync						int	double	bool	string															",
	"Formals							Variable MoreFormals	Variable MoreFormals	Variable MoreFormals	Variable MoreFormals										?					",
	"MoreFormals																				?			, Variable MoreFormals		",
	"StmtBlock	Sync						Sync	Sync	Sync	Sync	Sync	Sync	Sync	Sync							{ BlockBody }	Sync			",
	"BlockBody	Stmt BlockBody						Variable ; BlockBody	Variable ; BlockBody	Variable ; BlockBody	Variable ; BlockBody	Stmt BlockBody		Stmt BlockBody	Stmt BlockBody							Stmt BlockBody	?			",
	"Stmt	Ident ^A1 = Expr ^A6 ;						Sync	Sync	Sync	Sync	IfStmt	Sync	WhileStmt	ReturnStmt							StmtBlock	Sync			",
	"IfStmt	Sync						Sync	Sync	Sync	Sync	if ( Expr ) ^A7 Stmt ^A8 ElseStmt ^A9	Sync	Sync	Sync							Sync	Sync			",
	"ElseStmt	?						?	?	?	?	?	else Stmt	?	?							?	?			",
	"WhileStmt	Sync						Sync	Sync	Sync	Sync	Sync	Sync	^A10 while ( Expr ) ^A11 Stmt ^A12	Sync							Sync	Sync			",
	"ReturnStmt	Sync						Sync	Sync	Sync	Sync	Sync	Sync	Sync	return RetExpr ^A13 ;							Sync	Sync			",
	"RetExpr	Expr	Expr	Expr	Expr	Expr	Expr													Expr					?	",
	"Expr	Add Expr'	Add Expr'	Add Expr'	Add Expr'	Add Expr'	Add Expr'													Add Expr'			Sync	Sync	Sync	",
	"Expr'																	< RFExpr Expr'			?		Sync	?	?	",
	"RFExpr	Add ^A4	Add ^A4	Add ^A4	Add ^A4	Add ^A4	Add ^A4											Sync	 = Add ^A5	Add ^A4	Sync			Sync	Sync	",
	"Add	Mul Add'	Mul Add'	Mul Add'	Mul Add'	Mul Add'	Mul Add'											Sync		Mul Add'	Sync			Sync	Sync	",
	"Add'																+ Mul Add' ^A3	?			?			?	?	",
	"Mul	ExprBase Mul'	ExprBase Mul'	ExprBase Mul'	ExprBase Mul'	ExprBase Mul'	ExprBase Mul'										Sync	Sync		ExprBase Mul'	Sync			Sync	Sync	",
	"Mul'															* ExprBase Mul' ^A2	?	?			?			?	?	",
	"ExprBase	Ident ^A1 CallOrLValue	Constant ^A1	Constant ^A1	Constant ^A1	Constant ^A1	Constant ^A1									Sync	Sync	Sync		( Expr )	Sync			Sync	Sync	",
	"CallOrLValue															?	?	?		^A16 ( Actuals ) ^A17	?			?	?	",
	"Actuals	Expr ^A18 MoreActuals	Expr ^A18 MoreActuals	Expr ^A18 MoreActuals	Expr ^A18 MoreActuals	Expr ^A18 MoreActuals	Expr ^A18 MoreActuals													Expr ^A18 MoreActual	?					",
	"MoreActuals																				?			, Expr ^A18 MoreActuals		",
	"Constant		IntConst	DoubleConst	BoolConst	StringConst	null									Sync	Sync	Sync			Sync			Sync	Sync	"
};