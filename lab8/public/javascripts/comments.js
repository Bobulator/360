
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

	$("#getThem").click(function() {
      $.getJSON('comment', function(data) {
        console.log(data);
		var wordCountMap = new Map();
        var commentsDiv = "<ul>";
		var wordCountDiv = '<ul>';
        for(var comment in data) {
          com = data[comment];
          commentsDiv += "<li>Name: " + com.Name + " -- Comment: " + com.Comment + "</li>";
		  words = com.Comment.split(' ');
		  for (var i in words) {
			if (wordCountMap.has(words[i])) {
			  wordCountMap.set(words[i], wordCountMap.get(words[i])+1);
			} else if (words[i]) {
			  wordCountMap.set(words[i], 1);
			}
		  }
        }
		wordCountMap.forEach(function(value, key) {
		  wordCountDiv += "<li>" + key + " --  " + value + "</li>";
		}, wordCountMap);
        commentsDiv += "</ul>";
		wordCountDiv += "</ul>";
        $("#comments").html(commentsDiv);
		$("#wordCountsP").html("Word occurences from comments:");
		$("#wordCounts").html(wordCountDiv);
      })
    })
});
