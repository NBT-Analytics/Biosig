function [cc3,cc0,cbp,cc4]=make_cc7(fn,eegchan,trigchan,Fs)
% Builds a classifier based on AAR parameters from BCI recordings. 
%  The result includes also initial values for the next run. 
%
% CC=make_cc(fn,eegchan,trigchan,Fs)
%  
%  e.g. 
%  CC=make_cc('x21fb*')
%  CC=make_cc({'x21fb1.mat','x21fb2.mat'})
%  CC=make_cc(fn,[1,3],4,128)
%
% default: 
% 	eegchan=[1,3];
% 	trigchan=4;
% 	Fs=128;
%
% References:
% [1] Schl�gl A., Neuper C. Pfurtscheller G.
%   Estimating the mutual information of an EEG-based Brain-Computer-Interface.
%   Biomedizinische Technik 47(1-2): 3-8, 2002
% [2] A. Schl�gl, C. Keinrath, R. Scherer, G. Pfurtscheller,
%   Information transfer of an EEG-based Bran-computer interface.
%   Proceedings of the 1st International IEEE EMBS Conference on Neural Engineering, Capri, Italy, Mar 20-22, 2003. 
% [3] A. Schl�gl, D. Flotzinger and G. Pfurtscheller (1997)
%   Adaptive Autoregressive Modeling used for Single-trial EEG Classification
%   Biomedizinische Technik 42: (1997), 162-167. 
% [4] A. Schl�gl, C. Neuper and G. Pfurtscheller (1997)
%   Subject-specific EEG pattern during motor imagery
%   Proceedings of the 19th Annual International Conference if the IEEE Engineering in Medicine and Biology Society , vol 19, pp.1530-1532, 1997.
% [5] A. Schl�gl, K. Lugger and G. Pfurtscheller (1997)
%   Using Adaptive Autoregressive Parameters for a Brain-Computer-InterfaceExperiment,
%   Proceedings of the 19th Annual International Conference if the IEEE Engineering in Medicine and Biology Society ,vol 19 , pp.1533-1535, 1997.
%

%	Copyright (C) 1999-2002 by Alois Schloegl <a.schloegl@ieee.org>	
%	15.06.2002 Version 1.02

cname=computer;

if nargin<2,
	eegchan  = NaN;
end;
if nargin<3,
	trigchan = 4;
end;
if nargin<4,
	Fs = 128;
end;
 
M0  = 7;
MOP = 3;
uc  = 30:5:80;

[S,H] = sload(fn);
Fs = H.SampleRate;
cl = H.Classlabel(:);
if isnan(eegchan)
        eegchan=[1,H.NS-1];
end;

TRIG = gettrigger(S(:,trigchan));
if isfield(H,'TriggerOffset');
        TRIG = TRIG - H.TriggerOffset*Fs/1000;
        LEN = H.BCI.Paradigm.TrialDuration*Fs/1000;
else
        fprintf(1,'MAKE_CC7: assume trigger occures at t=2s \n');
        TRIG = TRIG - 2*Fs;
        LEN  = 9*Fs;
end;

if length(TRIG)~=length(cl);
        fprintf(2,'Attention: \n'),
end;
if isfield(H,'ArtifactSelection'),
        for k = find(H.ArtifactSelection)';
                S(TRIG(k)+(1:9*Fs),eegchan) = NaN;
        end;
end;
if isfield(H,'ArtifactSelection'),
        TRIG = TRIG(~H.ArtifactSelection);
        cl   = cl  (~H.ArtifactSelection);
end;

if length(TRIG)~=length(cl);
        fprintf(2,'number of Triggers (%i) does not fit size of class information (%i)',length(TRIG),length(cl));
	return;        
end;

if ~any(size(eegchan)==1)
	S = S(:,1:size(eegchan,1))*eegchan;
	eegchan=1:size(eegchan,2); 
end;

% Muscle Detection 
% [INI,s,E,arti] = detectmuscle2(S(:,1:3));

% find initial values 
randn('state',0);
[a0,A0] = getar0(S(:,eegchan),1:15,1000,Fs/2);
%save o3_init_A0 a0 A0
%load o3_init_A0 a0 A0
T = reshape((1:1152),16,1152/16)';
t0 = zeros(1152/16,1);
if ~isfield(H,'BCI')
        t0(33:60) = 1;   % look for interval [4s,7.5s]
else
        if isfield(H.BCI.Paradigm,'FallingPeriod')          % Basket Paradigm 
                T1 = H.BCI.Paradigm.FallingPeriod;
        elseif isfield(H.BCI.Paradigm,'FeedbackTiming')
                T1 = H.BCI.Paradigm.FeedbackTiming;
        end;
        %T1 = T1/1000*8;        % wrong !!!!
        T1 = T1/1000*Fs;        % correct !!!
        t0((T(:,1)>T1(1)) & (T(:,1)<T1(2))) = 1;
end;

t0 = logical(t0);

for p = 3;6;1:M0;
for k = 7;1:length(uc);
	UC0 = 2^(-uc(k)/8);
        ar = zeros(size(S,1),p*length(eegchan));
        e  = zeros(size(S,1),  length(eegchan));
        for ch = 1:length(eegchan),
                [ar(:,(1-p:0)+p*ch),e(:,ch),REV(ch)] = aar(S(:,eegchan(ch)), [2,3], p, UC0, a0{p},A0{p});
        end;

        [cc,Q,tsd,md] = findclassifier1(ar,TRIG, cl,T,t0,3);
        cc.MDA.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.MD2.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.LDA.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.GRB.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        
        tmp   = ar(~any(isnan(e),2),:);
        cc.C0 = covm(tmp,'E');
        cc.W0 = covm(diff(tmp),'M');
        cc.V  = meansq(e);
        for ch = 1:length(eegchan),
                li        = (1-p:0)+ch*p;
                cc.a0{ch} = cc.C0(1,li+1);
                cc.A0{ch} = cc.C0(li+1,li+1) - cc.a0{ch}'*cc.a0{ch};
	        cc.W{ch}  = cc.W0(li,li);
        end;
        
        cc.Method = 'aar';
        cc.T   = T;       
        cc.Fs  = Fs;
        cc.UC  = UC0;
        cc.p   = p;
        cc.MOP = p;
        cc.REV = REV;
        %cc.a0  = a0{p};
        %cc.A0  = A0{p};
        cc.EEGCHAN = eegchan;
        %cc.TRIGCHAN = trigchan;
        %CC{k+(p-1)*length(uc)}=cc;
end;
end;
cc0=cc ;


for p = 3;6;1:M0;
for k = 7;6;1:length(uc);
	UC0 = 2^(-uc(k)/8);
        ar = zeros(size(S,1),p*length(eegchan));
        e  = zeros(size(S,1),  length(eegchan));
        for ch = 1:length(eegchan),
                [ar(:,(1-p:0)+p*ch),e(:,ch),REV(ch)] = aar(S(:,eegchan(ch)), [0,0], p, UC0, cc0.a0{ch},cc0.A0{ch},cc0.W{ch},cc0.V(ch));
        end;
        
        [cc,Q,tsd,md] = findclassifier1(ar,TRIG, cl,T,t0,3);
        cc.MDA.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.MD2.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.LDA.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.GRB.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        
        tmp   = ar(~any(isnan(e),2),:);
        cc.C0 = covm(tmp,'E');
        cc.W0 = covm(diff(tmp),'M');
        cc.V  = meansq(e);
        for ch = 1:length(eegchan),
                li        = (1-p:0)+ch*p;
                cc.a0{ch} = cc.C0(1,li+1);
                cc.A0{ch} = cc.C0(li+1,li+1) - cc.a0{ch}'*cc.a0{ch};
	        cc.W{ch}  = cc.W0(li,li);
        end;
        
        cc.Method = 'aar';
        cc.T   = T;       
        cc.Fs  = Fs;
        cc.UC  = UC0;
        cc.p   = p;
        cc.MOP = p;
        cc.REV = REV;
        %cc.a0  = a0{p};
        %cc.A0  = A0{p};
        cc.EEGCHAN = eegchan;
        %cc.TRIGCHAN = trigchan;
        %CC{k+(p-1)*length(uc)}=cc;
end;
end;
cc3 = cc;


        uc=1/8;
        [cc,Q,tsd,md] = findclassifier1(filter(uc,[1,uc-1],ar),TRIG, cl,T,t0,3);
        cc.MDA.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.MD2.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.LDA.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.GRB.TSD.T = (min(T(:)):max(T(:)))'/Fs;
cc4=cc;        


s0 = S(:,eegchan);
s0(isnan(s0)) = 0;
if nargout>2,
        B1 = fir1(Fs,[10 12]/Fs*2);
        B2 = fir1(Fs,[16,24]/Fs*2);
        arc = log(filter(ones(Fs,1)/Fs,1,[filter(B1,1,s0),filter(B2,1,s0)].^2 ));

        [cc,Q,tsd,md] = findclassifier1(arc,TRIG, cl,T,t0,3);
        cc.MDA.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.MD2.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.LDA.TSD.T = (min(T(:)):max(T(:)))'/Fs;
        cc.GRB.TSD.T = (min(T(:)):max(T(:)))'/Fs;

        cc.EEGCHAN = eegchan;
        cc.BP.B1 = B1;
        cc.BP.B2 = B2;
end;
cbp = cc;
