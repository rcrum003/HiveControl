/*
 Popup JS v1 
 Used to center popups throughout site

Use this code in the page you want to include the popup.

 <p><a href="http://someurl" onclick="centeredPopup(this.href,'myWindow','700','300','yes');return false">Centered Popup</a></p>
*/



var popupWindow = null;

function centeredPopup(url,winName,w,h,scroll){
LeftPosition = (screen.width) ? (screen.width-w)/2 : 0;
TopPosition = (screen.height) ? (screen.height-h)/2 : 0;
settings =
'height='+h+',width='+w+',top='+TopPosition+',left='+LeftPosition+',scrollbars='+scroll+',resizable'
popupWindow = window.open(url,winName,settings)
}
