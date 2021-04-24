function [num] = tekread(filename)
%TEKREAD Imports channel data from Tektronix OpenChoice Desktop csv file
%    [NUM]=TEKREAD(FILENAME) reads a Tektronix Openchoice csv file, 
%    FILENAME. The numeric channel data (time and voltage) are returned in 
%    num as an array as shown below:
%
%    Col 1      Col 2       Col 3      Col 4
%    -----      -----       -----      -----
%    CH1-Time   CH1-Volts   CH2-Time   CH2-Volts
%
%    If Channel 2 data saved in the file, TEKREAD returns NaN for that
%    channel

%Created for ME479
%8/2014
%Modified for ME480 for use in Octave
%9/2019

%open file for reading 
fid = fopen(filename);

%read one line at a time
raw = fgetl(fid);

%create an index to increase the row number after reading each line
cnt = 0; 

%keep reading each line until end of file is reached
while raw~=-1 
    cnt = cnt+1;
    idx = strfind(raw,',');
    
    %extract only the numeric data between appropriate commas
    
    %If no ending comma is included, just use length of string as
    %terminator.  Otherwise, use ending comma.
    if length(idx)<5 
        endIdx = length(raw);
    else
        endIdx = idx(5)-1;
    end
    
    num(cnt,1) = str2double(raw(idx(3)+1:idx(4)-1));
    num(cnt,2) = str2double(raw(idx(4)+1:endIdx));
    
    %if 2nd channel not provided, stores NaNs
    if length(idx)>=10
        num(cnt,3) = str2double(raw(idx(9)+1:idx(10)-1));
        num(cnt,4) = str2double(raw(idx(10)+1:end));
    else   
        num(cnt,3) = NaN;
        num(cnt,4) = NaN;
    end %if
    
    raw = fgetl(fid);
end %while

%close file 
fclose(fid);

end

