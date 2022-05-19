%Generate tidal data for a year - COnfigured for the SimpleEstuary area
function tidalData(startYear)

%save a list of high and low tides, in format Year, month, day, day of week, time, height
filename = ["TideList" num2str(startYear) ".txt"];
fid = fopen (filename, "w");

for startMonth= 1:12

%find year start timestamp (or can we use unix timestamp internally??)
startTimestamp = 86400*(datenum(startYear,startMonth,1)-datenum(1970,1,1));
%end timestamp is end of first day of next month, minus one day
endTimestamp = 86400*(datenum(startYear,startMonth+1,1)-datenum(1970,1,1));

%Set timestep length
timestep = 60; %(s)

%set up list of times - add an extra step on each end so we can check for high/low at month start & end
time = [startTimestamp-timestep:timestep:endTimestamp+timestep];
timeHours = time / 3600;

%conversion constant
deg2rad = 2*pi/360;

constAmplitude = 3.08; %0th component
amplitude(1)=1.87;
amplitude(2)=0.63;
amplitude(3)=0.42;
amplitude(4)=0.21;
amplitude(5)=0.08;

speed(1)=28.9841042;
speed(2)=30;
speed(3)=28.4397295;
speed(4)=30.0821373;
speed(5)=57.4238337;

%phase is only needed for harmonics that share a common multiple of speed
phase(1)=-116.17;
phase(2)=-77.56;
phase(3)=66.31;
phase(4)=-267.8;
phase(5)=-30;

%convert speed from deg/hour to rad/hour and phase from deg to rad
speed = speed * deg2rad;
phase = phase * deg2rad;

%set up initial height (=constAmplitude, but same shape as timeHours)
height = timeHours * 0 + constAmplitude;

%calculate heights for timeHours
for tideComponent = 1:size(phase,2)
		height = height + amplitude(tideComponent) * cos(phase(tideComponent) + speed(tideComponent).*timeHours);
end
%height now contains height in m

%find max and min points
numberofHighLows = 0;
highLows=[];
%run through height list, excluding end points. Compare points with those before & after to find if higher or lower
for i = 2:size(height,2)-1
	%check for high tide
	if height(i) > height(i-1)  && height(i) > height(i+1)
			numberofHighLows++;
			%store height, time and high or low (+1 = high, -1 = low)
			highLows(numberofHighLows,1) = height(i);
			highLows(numberofHighLows,2) = time(i);
			highLows(numberofHighLows,3) = 1; 
			highLows(numberofHighLows,4) = startYear; %Year
			highLows(numberofHighLows,5) = startMonth; %Month
			day = 1+floor((time(i)-startTimestamp)/86400); %Day (1+ as first day is day 1, not 0)
			highLows(numberofHighLows,6) = day;
			hour = floor((time(i)-(startTimestamp+(day-1)*86400))/3600); %timestamp less start of month, less how far into the month the day start is
			highLows(numberofHighLows,7) = hour;
			minute = floor((time(i)-(startTimestamp+(day-1)*86400+hour*3600))/60);
			highLows(numberofHighLows,8) = minute;
			highLows(numberofHighLows,9) = weekday((time(i)/86400)+datenum(1970,1,1));
	end
	
	%check for low tide
	if height(i) < height(i-1)  && height(i) < height(i+1)
			numberofHighLows++;
			%store height, time and high or low (+1 = high, -1 = low)
			highLows(numberofHighLows,1) = height(i);
			highLows(numberofHighLows,2) = time(i);
			highLows(numberofHighLows,3) = -1; 
			highLows(numberofHighLows,4) = startYear; %Year
			highLows(numberofHighLows,5) = startMonth; %Month
			day = 1+floor((time(i)-startTimestamp)/86400); %Day (1+ as first day is day 1, not 0)
			highLows(numberofHighLows,6) = day;
			hour = floor((time(i)-(startTimestamp+(day-1)*86400))/3600); %timestamp less start of month, less how far into the month the day start is
			highLows(numberofHighLows,7) = hour;
			minute = floor((time(i)-(startTimestamp+(day-1)*86400+hour*3600))/60);
			highLows(numberofHighLows,8) = minute;
			highLows(numberofHighLows,9) = weekday((time(i)/86400)+datenum(1970,1,1));
	end
end

%Plot tide, showing highs and lows
%plot(time, height,highLows(:,2),highLows(:,1),"+")

%set up null initial conditions
currentDay = 0;

%print year
if startMonth==1
 fprintf(fid,"Year: %i\n",startYear);
end

%get month name
switch startMonth
	case 1
		monthname =   "January\n\n\n\n";
	case 2
		monthname = "\nFebruary\n\n\n\n";
	case 3
		monthname = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nMarch\n\n\n\n";
	case 4
		monthname = "\nApril\n\n\n\n";
	case 5
		monthname = "\n\n\n\n\n\n\nMay\n\n\n\n";
	case 6
		monthname = "\nJune\n\n\n\n";
	case 7
		monthname = "\n\n\n\n\n\n\nJuly\n\n\n\n";
	case 8
		monthname = "\nAugust\n\n\n\n";
	case 9
		monthname = "\nSeptember\n\n\n\n";
	case 10
		monthname = "\n\n\n\n\n\n\nOctober\n\n\n\n";
	case 11
		monthname = "\nNovember\n\n\n\n";
	case 12
		monthname = "\n\n\n\n\n\n\nDecember\n\n\n\n";
endswitch
fprintf(fid,"%s",monthname);

tidesPlotted = 0;

for i = 1:size(highLows,1)
	if currentDay != highLows(i,6)
		currentDay = highLows(i,6);
		
		%get day name
		switch highLows(i,9) %the integer weekday, sunday = 1
			case 1
				dayname = "SU";
			case 2
				dayname = "M";
			case 3
				dayname = "TU";
			case 4
				dayname = "W";
			case 5
				dayname = "TH";
			case 6
				dayname = "F";
			case 7
				dayname = "SA";
		endswitch
		
		if tidesPlotted < 4
			fprintf(fid,"\n");
		end
		
		fprintf(fid,"\n");
		fprintf(fid,"%i - %s\n",currentDay, dayname);
		
		tidesPlotted = 0;
		
	end
	
	%print actual tidal info
	fprintf(fid,"%02i%02i %.2f\n",highLows(i,7), highLows(i,8), highLows(i,1));
	tidesPlotted = tidesPlotted+1;
	
end
end

fclose (fid);

end
