(function($){
	$(function() {
		$("#choose-date").datepicker({
   			onSelect: function(dateText, inst) {
   				$("#date-details").html('Loading schedule for <strong>' + dateText + '</strong>...');
   				path = window.location.pathname.split("/");
   				$("#date-details").load("http://www.elitetutorsonline.com/callback/schedule", {date:dateText,user:path[2]});
   			}
		});
	});
})(jQuery);