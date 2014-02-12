%%
%% This is file `eeict.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% Author: Martin Drahansky, drahan@fit.vutbr.cz
%%
%% eeict.dtx  (with options: `class')
%% 
%% 

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{eeict}% 
    [2002/11/11 Dokumentarni trida pro prispevky do sborniku EEICT]
\usepackage[T1]{fontenc} 
\LoadClass[a4paper,onecolumn,11pt]{article}
\setlength{\headheight}{0cm}
\setlength{\headsep}{0cm}    
\usepackage[top=2.5cm, bottom=3.2cm, left=3cm, right=2.5cm]{geometry} 
\setlength{\parskip}{6pt}
\usepackage{mathptm} 
\renewcommand\rmdefault{ptm} 
\renewcommand\ttdefault{pcr} 

\ifx\pdfoutput\undefined  % nejedeme pod pdftexem
  \usepackage[czech,english]{babel}
  \main@language{english}
  \usepackage[latin2]{inputenc}
  \usepackage{graphics}
  \usepackage{epsfig}
\else % je to pdftex !
  \usepackage[czech,english]{babel}
  \main@language{english}
  \usepackage[latin2]{inputenc}
  \usepackage[pdftex]{graphicx}
\fi

\pagestyle{empty}
\setlength{\parindent}{0cm} 
\let\@afterindentfalse\@afterindenttrue 
\@afterindenttrue 

\newcommand\supervisor[1]{\def\@skolitel{Supervised by: #1}}
\newcommand\programme[1]{\def\@program{#1}}
\newcommand\emails[1]{\def\@emajls{E-mail: #1}}
\newcommand\emailv[1]{\def\@emajlv{E-mail: #1}}
\renewcommand\abstract[1]{\def\@abstract{{\bf Abstract}: #1}}
\newcommand\keywords[1]{\def\@keywords{{\bf Keywords}: #1}}
\renewcommand\maketitle{
  \selectlanguage{english}
  \begin{center}  
    {\Large\textbf{\MakeUppercase{\@title}} }\\
    \vspace{24pt}
    {\normalsize\textbf\@author}\\  
    {\footnotesize\@program}\\
    {\footnotesize\@emajls}\\
    \vspace{24pt}
    \@skolitel\\
    {\footnotesize\@emajlv}\\
    \vspace{18pt}
  \end{center}
  {\normalsize\@abstract}
     
  {\normalsize\@keywords}
} 

\renewcommand\section{\@startsection{section}{1}{-0.5cm}%
                                   {-2.5ex \@plus -1ex \@minus -.2ex}%
                                   {0.2ex \@plus.2ex}%
                                   {\normalfont\normalsize\bfseries\MakeUppercase}}
\renewcommand\subsection{\@startsection{subsection}{2}{\z@}% 
                                     {-2.25ex\@plus -1ex \@minus -.2ex}%
                                     {0.2ex \@plus .2ex}%
                                     {\normalfont\normalsize\bfseries\scshape}}
\renewcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}%
                                     {-2.25ex\@plus -1ex \@minus -.2ex}%
                                     {0.2ex \@plus .2ex}%
                                     {\normalfont\normalsize\scshape}}

\def\sktitle{Supervised by: }
\def\@skolitel{the work has no suppervisor}
\def\uv#1{\iflanguage{english}{``#1''}%
                              {\quotedblbase #1\textquotedblleft}}%
\catcode`\"=\active
\def"{\bgroup
    \iflanguage{english}%
        {``\def"{''\egroup}}%
        {\quotedblbase\def"{\textquotedblleft\egroup}}}
\endinput

%%
%% End of file `eeict.cls'.
