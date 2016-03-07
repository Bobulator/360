
$(document).ready(function(){
    $("#serialize").click(function(){
        var myobj = {Name:$("#Name").val(),Comment:$("#Comment").val()};
        jobj = JSON.stringify(myobj);
        $("#json").text(jobj);
    	var url = "comment";
		$.ajax({
  		url:url,
  		type: "POST",
  		data: jobj,
  		contentType: "application/json; charset=utf-8",
  		success: function(data,textStatus) {
      		$("#done").html(textStatus);
  		}
		})
	});
});
