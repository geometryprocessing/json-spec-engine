## _*_ coding: utf-8 _*_
<!DOCTYPE HTML>

<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.0.0/dist/css/bootstrap.min.css" integrity="sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm" crossorigin="anonymous">
<script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/popper.js@1.12.9/dist/umd/popper.min.js" integrity="sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@4.0.0/dist/js/bootstrap.min.js" integrity="sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl" crossorigin="anonymous"></script>

<%def name="render_pointer(pointer_name)">
	${make_rules(rs = ijson.rules_from_pointer(pointer_name))}
</%def>

<%def name="make_rules(rs)">
		<% ## get a unique id
		attributes['count'] = attributes['count'] + 1
		local_count = attributes['count']
		%>
		% if len(rs) == 1: ## single rules for a pointer use a cell
			${make_rule(r=rs[0])}
		% endif
		% if len(rs) > 1: ## multiple rules use a collapsible block
		<div id="accordion${local_count}">
  			<div class="card">
    			<div class="card-header" id="heading${local_count}">
      				<h5 class="mb-0">
						<button class="btn btn-link" data-toggle="collapse" data-target="#collapse${local_count}" aria-expanded="true" aria-controls="collapse${local_count}">
						${rs[0]['pointer']}
						</button>
      				</h5>
    			</div>
    			<div id="collapse${local_count}" class="collapse show" aria-labelledby="heading${local_count}" data-parent="#accordion${local_count}">
					% for r in rs:
					${make_rule(r=rs[0])}
					% endfor
    			</div>
  			</div>
		</div>
		% endif
</%def>

<%def name="make_rule(r)">
		<%
		attributes['count'] = attributes['count'] + 1
		local_count = attributes['count']
		%>
		% if r['type'] == "object": ## Object
		<div id="accordion${local_count}">
  			<div class="card">
    			<div class="card-header" id="heading${local_count}">
      				<h5 class="mb-0">
						<button class="btn btn-link" data-toggle="collapse" data-target="#collapse${local_count}" aria-expanded="true" aria-controls="collapse${local_count}">
						${r['pointer']}, Object
						</button>
      				</h5>
    			</div>
    			<div id="collapse${local_count}" class="collapse hide" aria-labelledby="heading${local_count}" data-parent="#accordion${local_count}">
					
					% if 'required' in r:
					<div class="container">
						<div class="row">
							<div class="auto">
							Required
							</div>
							<div class="col-sm">
								% for fname in r['required']:
								${render_pointer(pointer_name = (r['pointer']+fname) if r['pointer'] == '/' else (r['pointer']+'/'+fname))}
								% endfor
						    </div>
  						</div>
					</div>
					% endif
					% if 'optional' in r:
					<div class="container">
						<div class="row">
							<div class="auto">
							Optional
							</div>
							<div class="col-sm">
								% for fname in r['optional']:
								${render_pointer(pointer_name = (r['pointer']+fname) if r['pointer'] == '/' else (r['pointer']+'/'+fname))}
								% endfor
						    </div>
  						</div>
					</div>
					% endif

    			</div>
  			</div>
		</div>
		% endif
		% if r['type'] == "list": ## List
		${render_pointer(pointer_name = (r['pointer']+'*') if r['pointer'] == '/' else (r['pointer']+'/'+'*'))}
		% endif
</%def>

<html>
	<body>
	${render_pointer(pointer_name='/')}
	</body>
</html>