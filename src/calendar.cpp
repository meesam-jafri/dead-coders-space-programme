#include "game.hpp"
namespace mission {
std::string Money(int amount){
    std::string text=std::to_string(std::abs(amount));
    for(int i=int(text.size())-3;i>0;i-=3)text.insert(i,",");
    return std::string(amount<0?"-$":"$")+text;
}
std::string Clock(float seconds){
    int total=int(std::max(0.0f,seconds));
    char text[32];std::snprintf(text,sizeof(text),"%02d:%02d:%02d",total/3600,(total/60)%60,total%60);
    return text;
}
int DaysInMonth(int year,int month){
    if(month<1||month>12)return 0;
    constexpr int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
    bool leap=year%4==0&&(year%100!=0||year%400==0);
    return days[month-1]+(month==2&&leap?1:0);
}
// Gregorian weekday calculation; return Monday=0 ... Sunday=6.
int FirstWeekday(int year,int month){
    constexpr int offsets[]={0,3,2,5,0,3,5,1,4,6,2,4};
    if(month<1||month>12)return 0;
    int y=year-(month<3?1:0);
    return ((y+y/4-y/100+y/400+offsets[month-1]+1)%7+6)%7;
}
LaunchWeather WeatherFor(int year,int month,int day){
    if(day<1||day>DaysInMonth(year,month))return {99,true,false,0};
    int pattern=((day-1+(year-2026)*12+month-11)%6+6)%6;
    constexpr int winds[]={32,8,12,18,6,10};
    return {winds[pattern],pattern==2,pattern!=4,pattern==3?2:pattern==5?4:0};
}
std::string LaunchWeather::Summary()const{
    if(lightning)return "Lightning risk: postpone launch.";
    if(windKph>=25)return "Strong winds: launch is blocked.";
    if(!transfer)return "Clear sky, but no transfer window.";
    if(delay)return "Flyable. Arrival uses extra battery.";
    return "Clear sky. Favorable transfer window.";
}
LaunchWeather SelectedWeather(const GameState& g){return WeatherFor(g.launchYear,g.launchMonth,g.date+1);}
std::string DateLabel(const GameState& g){
    if(g.date<0)return "No date selected";
    char text[32];std::snprintf(text,sizeof(text),"%04d-%02d-%02d",g.launchYear,g.launchMonth,g.date+1);return text;
}
}